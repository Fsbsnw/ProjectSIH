#include "PFDialogueSubsystem.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/001_Data/000_Definitions/PFDialogueDefinition.h"

void UPFDialogueSubsystem::Deinitialize()
{
	ClearRuntimeState();
	m_OnLinePresented.Clear();
	m_OnChoicesPresented.Clear();
	m_OnActionRequested.Clear();
	m_OnCompleted.Clear();
	m_OnFailed.Clear();
	m_OnCancelled.Clear();

	Super::Deinitialize();
}

EPFDialogueInputResult UPFDialogueSubsystem::StartDialogue(
	UPFDialogueDefinition* DialogueDefinition)
{
	if (IsInputReentrant())
	{
		return EPFDialogueInputResult::ReentrantInput;
	}

	if (m_bDialogueActive)
	{
		return EPFDialogueInputResult::AlreadyActive;
	}

	FString ValidationError;
	if (!IsValid(DialogueDefinition)
		|| !DialogueDefinition->ValidateRuntimeData(&ValidationError))
	{
		PF_LOG(
			TEXT("Dialogue Definition is invalid. Reason=%s"),
			*ValidationError);
		return EPFDialogueInputResult::InvalidDialogueDefinition;
	}

	ClearRuntimeState();
	m_ActiveDefinition = DialogueDefinition;
	m_bDialogueActive = true;

	TGuardValue<bool> ProcessingGuard(m_bProcessingDialogue, true);
	if (!EnterNode(DialogueDefinition->GetEntryNodeID()))
	{
		FailActiveDialogue();
	}

	return EPFDialogueInputResult::Accepted;
}

EPFDialogueInputResult UPFDialogueSubsystem::ContinueDialogue()
{
	if (IsInputReentrant())
	{
		return EPFDialogueInputResult::ReentrantInput;
	}

	if (!m_bDialogueActive)
	{
		return EPFDialogueInputResult::InactiveDialogue;
	}

	if (m_CurrentStep != EPFDialogueStep::Line)
	{
		return EPFDialogueInputResult::WrongStep;
	}

	const FPFDialogueRuntimeNode* CurrentNode = GetCurrentNode();
	if (!CurrentNode)
	{
		FailActiveDialogue();
		return EPFDialogueInputResult::InvalidNodeID;
	}

	TGuardValue<bool> ProcessingGuard(m_bProcessingDialogue, true);
	++m_CurrentLineIndex;

	if (CurrentNode->m_Lines.IsValidIndex(m_CurrentLineIndex))
	{
		if (!PresentCurrentLine())
		{
			FailActiveDialogue();
		}
		return EPFDialogueInputResult::Accepted;
	}

	m_CurrentLinePresentation = {};
	if (!CurrentNode->m_Choices.IsEmpty())
	{
		if (!PresentCurrentChoices())
		{
			FailActiveDialogue();
		}
		return EPFDialogueInputResult::Accepted;
	}

	m_CurrentStep = EPFDialogueStep::TransitionPending;
	if (!EnterNode(CurrentNode->m_NextNodeID))
	{
		FailActiveDialogue();
	}

	return EPFDialogueInputResult::Accepted;
}

EPFDialogueInputResult UPFDialogueSubsystem::SelectChoice(
	const FName ChoiceID)
{
	if (IsInputReentrant())
	{
		return EPFDialogueInputResult::ReentrantInput;
	}

	if (!m_bDialogueActive)
	{
		return EPFDialogueInputResult::InactiveDialogue;
	}

	if (ChoiceID.IsNone())
	{
		return EPFDialogueInputResult::InvalidChoiceID;
	}

	if (m_CurrentStep != EPFDialogueStep::Choices)
	{
		return m_PreviouslyPresentedChoiceIDs.Contains(ChoiceID)
			? EPFDialogueInputResult::StaleChoice
			: EPFDialogueInputResult::WrongStep;
	}

	const FPFDialogueChoice* SelectedChoice =
		m_CurrentChoicePresentation.m_Choices.FindByPredicate(
			[ChoiceID](const FPFDialogueChoice& Choice)
			{
				return Choice.m_ChoiceID == ChoiceID;
			});

	if (!SelectedChoice)
	{
		return m_PreviouslyPresentedChoiceIDs.Contains(ChoiceID)
			? EPFDialogueInputResult::StaleChoice
			: EPFDialogueInputResult::InvalidChoiceID;
	}

	const FName NextNodeID = SelectedChoice->m_NextNodeID;
	ArchiveCurrentChoices();
	m_CurrentChoicePresentation = {};
	m_CurrentStep = EPFDialogueStep::TransitionPending;

	TGuardValue<bool> ProcessingGuard(m_bProcessingDialogue, true);
	if (!EnterNode(NextNodeID))
	{
		FailActiveDialogue();
	}

	return EPFDialogueInputResult::Accepted;
}

EPFDialogueInputResult UPFDialogueSubsystem::CompleteAction(
	const FName ActionNodeID,
	const FName ResultID)
{
	if (IsInputReentrant())
	{
		return EPFDialogueInputResult::ReentrantInput;
	}

	if (!m_bDialogueActive)
	{
		return EPFDialogueInputResult::InactiveDialogue;
	}

	if (m_CurrentStep != EPFDialogueStep::Action)
	{
		return ActionNodeID == m_LastCompletedActionNodeID
			? EPFDialogueInputResult::StaleAction
			: EPFDialogueInputResult::WrongStep;
	}

	if (ActionNodeID.IsNone()
		|| ActionNodeID != m_CurrentNodeID)
	{
		return EPFDialogueInputResult::StaleAction;
	}

	if (ResultID.IsNone())
	{
		return EPFDialogueInputResult::InvalidActionResultID;
	}

	const FPFDialogueRuntimeNode* CurrentNode = GetCurrentNode();
	if (!CurrentNode)
	{
		FailActiveDialogue();
		return EPFDialogueInputResult::InvalidNodeID;
	}

	const FPFDialogueActionBranch* SelectedBranch =
		CurrentNode->m_Action.m_Branches.FindByPredicate(
			[ResultID](const FPFDialogueActionBranch& Branch)
			{
				return Branch.m_ResultID == ResultID;
			});
	if (!SelectedBranch)
	{
		return EPFDialogueInputResult::ActionBranchNotFound;
	}

	const FName NextNodeID = SelectedBranch->m_NextNodeID;
	if (!IsValid(m_ActiveDefinition)
		|| !m_ActiveDefinition->FindNode(NextNodeID))
	{
		FailActiveDialogue();
		return EPFDialogueInputResult::InvalidNodeID;
	}

	m_LastCompletedActionNodeID = ActionNodeID;
	m_CurrentActionPresentation = {};
	m_CurrentStep = EPFDialogueStep::TransitionPending;

	TGuardValue<bool> ProcessingGuard(m_bProcessingDialogue, true);
	if (!EnterNode(NextNodeID))
	{
		FailActiveDialogue();
	}

	return EPFDialogueInputResult::Accepted;
}

EPFDialogueInputResult UPFDialogueSubsystem::CancelDialogue()
{
	if (IsInputReentrant())
	{
		return EPFDialogueInputResult::ReentrantInput;
	}

	if (!m_bDialogueActive)
	{
		return EPFDialogueInputResult::InactiveDialogue;
	}

	ClearRuntimeState();

	TGuardValue<bool> BroadcastGuard(m_bBroadcastingDelegate, true);
	m_OnCancelled.Broadcast();
	return EPFDialogueInputResult::Accepted;
}

bool UPFDialogueSubsystem::IsDialogueActive() const
{
	return m_bDialogueActive;
}

bool UPFDialogueSubsystem::HasCurrentLine() const
{
	return m_CurrentStep == EPFDialogueStep::Line;
}

bool UPFDialogueSubsystem::HasCurrentChoices() const
{
	return m_CurrentStep == EPFDialogueStep::Choices;
}

bool UPFDialogueSubsystem::IsWaitingForAction() const
{
	return m_CurrentStep == EPFDialogueStep::Action;
}

const FPFDialogueLinePresentation&
UPFDialogueSubsystem::GetCurrentLinePresentation() const
{
	return m_CurrentLinePresentation;
}

const FPFDialogueChoicePresentation&
UPFDialogueSubsystem::GetCurrentChoicePresentation() const
{
	return m_CurrentChoicePresentation;
}

const FPFDialogueActionPresentation&
UPFDialogueSubsystem::GetCurrentActionPresentation() const
{
	return m_CurrentActionPresentation;
}

FPFDialogueLinePresented&
UPFDialogueSubsystem::GetLinePresentedDelegate()
{
	return m_OnLinePresented;
}

FPFDialogueChoicesPresented&
UPFDialogueSubsystem::GetChoicesPresentedDelegate()
{
	return m_OnChoicesPresented;
}

FPFDialogueActionRequested&
UPFDialogueSubsystem::GetActionRequestedDelegate()
{
	return m_OnActionRequested;
}

FPFDialogueCompleted&
UPFDialogueSubsystem::GetCompletedDelegate()
{
	return m_OnCompleted;
}

FPFDialogueFailed&
UPFDialogueSubsystem::GetFailedDelegate()
{
	return m_OnFailed;
}

FPFDialogueCancelled&
UPFDialogueSubsystem::GetCancelledDelegate()
{
	return m_OnCancelled;
}

bool UPFDialogueSubsystem::EnterNode(const FName NodeID)
{
	TSet<FName> VisitedImmediateNodes;
	FName NextNodeID = NodeID;

	while (m_bDialogueActive)
	{
		if (NextNodeID.IsNone()
			|| VisitedImmediateNodes.Contains(NextNodeID))
		{
			return false;
		}

		VisitedImmediateNodes.Add(NextNodeID);
		m_CurrentNodeID = NextNodeID;
		m_CurrentLineIndex = INDEX_NONE;

		const FPFDialogueRuntimeNode* Node = GetCurrentNode();
		if (!Node)
		{
			return false;
		}

		switch (Node->m_NodeType)
		{
		case EPFDialogueNodeType::Dialogue:
			if (!Node->m_Lines.IsEmpty())
			{
				m_CurrentLineIndex = 0;
				return PresentCurrentLine();
			}

			if (!Node->m_Choices.IsEmpty())
			{
				return PresentCurrentChoices();
			}

			NextNodeID = Node->m_NextNodeID;
			break;

		case EPFDialogueNodeType::Action:
			return RequestCurrentAction();

		case EPFDialogueNodeType::End:
			CompleteActiveDialogue();
			return true;
		}
	}

	return true;
}

bool UPFDialogueSubsystem::PresentCurrentLine()
{
	const FPFDialogueRuntimeNode* Node = GetCurrentNode();
	if (!Node
		|| !Node->m_Lines.IsValidIndex(m_CurrentLineIndex))
	{
		return false;
	}

	ArchiveCurrentChoices();
	m_CurrentChoicePresentation = {};
	m_CurrentActionPresentation = {};
	m_CurrentLinePresentation.m_Line = Node->m_Lines[m_CurrentLineIndex];
	m_CurrentLinePresentation.m_LineIndex = m_CurrentLineIndex;
	m_CurrentLinePresentation.m_LineCount = Node->m_Lines.Num();
	m_CurrentStep = EPFDialogueStep::Line;

	TGuardValue<bool> BroadcastGuard(m_bBroadcastingDelegate, true);
	m_OnLinePresented.Broadcast(m_CurrentLinePresentation);
	return true;
}

bool UPFDialogueSubsystem::PresentCurrentChoices()
{
	const FPFDialogueRuntimeNode* Node = GetCurrentNode();
	if (!Node || Node->m_Choices.IsEmpty())
	{
		return false;
	}

	ArchiveCurrentChoices();
	m_CurrentLinePresentation = {};
	m_CurrentActionPresentation = {};
	m_CurrentChoicePresentation.m_Choices = Node->m_Choices;
	m_CurrentStep = EPFDialogueStep::Choices;

	TGuardValue<bool> BroadcastGuard(m_bBroadcastingDelegate, true);
	m_OnChoicesPresented.Broadcast(m_CurrentChoicePresentation);
	return true;
}

bool UPFDialogueSubsystem::RequestCurrentAction()
{
	const FPFDialogueRuntimeNode* Node = GetCurrentNode();
	if (!Node
		|| !Node->m_Action.IsValid()
		|| !m_OnActionRequested.IsBound())
	{
		return false;
	}

	ArchiveCurrentChoices();
	m_CurrentLinePresentation = {};
	m_CurrentChoicePresentation = {};
	m_CurrentActionPresentation.m_NodeID = m_CurrentNodeID;
	m_CurrentActionPresentation.m_Action = Node->m_Action;
	m_LastCompletedActionNodeID = NAME_None;
	m_CurrentStep = EPFDialogueStep::Action;

	TGuardValue<bool> BroadcastGuard(m_bBroadcastingDelegate, true);
	m_OnActionRequested.Broadcast(m_CurrentActionPresentation);
	return true;
}

const FPFDialogueRuntimeNode*
UPFDialogueSubsystem::GetCurrentNode() const
{
	return IsValid(m_ActiveDefinition)
		? m_ActiveDefinition->FindNode(m_CurrentNodeID)
		: nullptr;
}

void UPFDialogueSubsystem::ArchiveCurrentChoices()
{
	for (const FPFDialogueChoice& Choice
		: m_CurrentChoicePresentation.m_Choices)
	{
		if (!Choice.m_ChoiceID.IsNone())
		{
			m_PreviouslyPresentedChoiceIDs.Add(Choice.m_ChoiceID);
		}
	}
}

void UPFDialogueSubsystem::CompleteActiveDialogue()
{
	ClearRuntimeState();

	TGuardValue<bool> BroadcastGuard(m_bBroadcastingDelegate, true);
	m_OnCompleted.Broadcast();
}

void UPFDialogueSubsystem::FailActiveDialogue()
{
	ClearRuntimeState();

	TGuardValue<bool> BroadcastGuard(m_bBroadcastingDelegate, true);
	m_OnFailed.Broadcast();
}

void UPFDialogueSubsystem::ClearRuntimeState()
{
	m_ActiveDefinition = nullptr;
	m_CurrentNodeID = NAME_None;
	m_LastCompletedActionNodeID = NAME_None;
	m_CurrentLineIndex = INDEX_NONE;
	m_CurrentLinePresentation = {};
	m_CurrentChoicePresentation = {};
	m_CurrentActionPresentation = {};
	m_PreviouslyPresentedChoiceIDs.Reset();
	m_CurrentStep = EPFDialogueStep::None;
	m_bDialogueActive = false;
}

bool UPFDialogueSubsystem::IsInputReentrant() const
{
	return m_bProcessingDialogue
		|| m_bBroadcastingDelegate;
}
