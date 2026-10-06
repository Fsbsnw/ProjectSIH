#include "PFDialogueGraphExecutor.h"

#include "Engine/DataTable.h"
#include "PFDialogueGraphAsset.h"
#include "PFDialogueGraphTypes.h"

namespace
{
const FName PN_Out(TEXT("Out"));
const FName PN_Cancelled(TEXT("Cancelled"));
const FName PN_Correct(TEXT("Correct"));
const FName PN_Incorrect(TEXT("Incorrect"));
const FName PN_Failed(TEXT("Failed"));

FName MakeIndexedOutputName(const TCHAR* Prefix, int32 Index)
{
	return FName(*FString::Printf(TEXT("%s_%d"), Prefix, Index));
}

TArray<FText> GetEffectiveChoices(const FPFDialogueTableRow& Row)
{
	if (!Row.Choices.IsEmpty())
	{
		return Row.Choices;
	}

	TArray<FText> Choices;
	if (!Row.ChoiceA.IsEmpty())
	{
		Choices.Add(Row.ChoiceA);
	}
	if (!Row.ChoiceB.IsEmpty())
	{
		Choices.Add(Row.ChoiceB);
	}
	return Choices;
}
}

bool UPFDialogueGraphExecutor::Start(UPFDialogueGraphAsset* DialogueAsset)
{
	Cancel();
	if (!IsValid(DialogueAsset)
		|| !DialogueAsset->GetStartNodeID().IsValid()
		|| !DialogueAsset->GetDialogueDataTable())
	{
		SetFailed();
		return false;
	}

	m_DialogueAsset = DialogueAsset;
	return EnterNode(DialogueAsset->GetStartNodeID());
}

bool UPFDialogueGraphExecutor::ContinueDialogue()
{
	return m_ExecutionState == EPFDialogueExecutionState::WaitingForContinue
		&& AdvanceFromCurrentNode(PN_Out);
}

bool UPFDialogueGraphExecutor::SelectInlineChoice(int32 ChoiceIndex)
{
	if (m_ExecutionState != EPFDialogueExecutionState::WaitingForInlineChoice
		|| !m_CurrentInlineChoice.Choices.IsValidIndex(ChoiceIndex))
	{
		return false;
	}

	return AdvanceFromCurrentNode(MakeIndexedOutputName(TEXT("Choice"), ChoiceIndex));
}

bool UPFDialogueGraphExecutor::SubmitExternalResponse(
	const FPFDialogueExternalSelectionResponse& Response)
{
	if (m_ExecutionState != EPFDialogueExecutionState::WaitingForExternalResponse
		|| !Response.RequestID.IsValid()
		|| Response.RequestID != m_PendingExternalRequest.RequestID)
	{
		return false;
	}

	const FPFDialogueRuntimeNode* CurrentNode = m_DialogueAsset
		? m_DialogueAsset->FindRuntimeNode(m_CurrentNodeID)
		: nullptr;
	if (!CurrentNode)
	{
		SetFailed();
		return false;
	}

	FName OutputName = PN_Failed;
	if (CurrentNode->NodeType == EPFDialogueGraphNodeType::ClaimSelectionRequest)
	{
		if (Response.ResponseType == EPFDialogueExternalResponseType::Selected)
		{
			const int32 SelectionIndex = CurrentNode->OptionContentIDs.IndexOfByKey(
				Response.SelectedContentID);
			if (SelectionIndex == INDEX_NONE)
			{
				return false;
			}

			m_ActiveClaimID = Response.SelectedContentID;
			m_LastSelectedContentID = Response.SelectedContentID;
			OutputName = MakeIndexedOutputName(TEXT("Selection"), SelectionIndex);
		}
		else if (Response.ResponseType == EPFDialogueExternalResponseType::Cancelled)
		{
			m_ActiveClaimID = FGameplayTag();
			OutputName = PN_Cancelled;
		}
	}
	else if (CurrentNode->NodeType == EPFDialogueGraphNodeType::EvidenceSelectionRequest)
	{
		m_LastSelectedContentID = Response.SelectedContentID;
		switch (Response.ResponseType)
		{
		case EPFDialogueExternalResponseType::Correct:
			OutputName = PN_Correct;
			break;
		case EPFDialogueExternalResponseType::Incorrect:
			OutputName = PN_Incorrect;
			break;
		case EPFDialogueExternalResponseType::Cancelled:
			OutputName = PN_Cancelled;
			break;
		default:
			OutputName = PN_Failed;
			break;
		}
	}
	else
	{
		return false;
	}

	return AdvanceFromCurrentNode(OutputName);
}

void UPFDialogueGraphExecutor::Cancel()
{
	m_DialogueAsset = nullptr;
	m_CurrentNodeID.Invalidate();
	m_ExecutionState = EPFDialogueExecutionState::Idle;
	m_CurrentLine = FPFDialogueLinePresentation();
	m_CurrentInlineChoice = FPFDialogueInlineChoicePresentation();
	m_PendingExternalRequest = FPFDialogueExternalSelectionRequest();
	m_ActiveClaimID = FGameplayTag();
	m_LastSelectedContentID = FGameplayTag();
}

bool UPFDialogueGraphExecutor::EnterNode(const FGuid& NodeID)
{
	if (!m_DialogueAsset)
	{
		SetFailed();
		return false;
	}

	FGuid NextNodeID = NodeID;
	for (int32 StepCount = 0; StepCount < 256; ++StepCount)
	{
		const FPFDialogueRuntimeNode* Node = m_DialogueAsset->FindRuntimeNode(NextNodeID);
		if (!Node)
		{
			SetFailed();
			return false;
		}

		m_CurrentNodeID = Node->NodeID;
		switch (Node->NodeType)
		{
		case EPFDialogueGraphNodeType::Root:
		{
			const FPFDialogueGraphTransition* Transition = Node->Transitions.FindByPredicate(
				[](const FPFDialogueGraphTransition& Item)
				{
					return Item.OutputName == PN_Out;
				});
			if (!Transition || !Transition->TargetNodeID.IsValid())
			{
				SetFailed();
				return false;
			}
			NextNodeID = Transition->TargetNodeID;
			break;
		}
		case EPFDialogueGraphNodeType::Dialogue:
			return BuildLinePresentation(*Node);
		case EPFDialogueGraphNodeType::InlineChoice:
			return BuildInlineChoicePresentation(*Node);
		case EPFDialogueGraphNodeType::ClaimSelectionRequest:
		case EPFDialogueGraphNodeType::EvidenceSelectionRequest:
			return BuildExternalRequest(*Node);
		case EPFDialogueGraphNodeType::End:
			m_ExecutionState = EPFDialogueExecutionState::Completed;
			OnDialogueCompleted.Broadcast();
			return true;
		default:
			SetFailed();
			return false;
		}
	}

	SetFailed();
	return false;
}

bool UPFDialogueGraphExecutor::AdvanceFromCurrentNode(FName OutputName)
{
	const FPFDialogueRuntimeNode* Node = m_DialogueAsset
		? m_DialogueAsset->FindRuntimeNode(m_CurrentNodeID)
		: nullptr;
	const FPFDialogueGraphTransition* Transition = Node
		? Node->Transitions.FindByPredicate(
			[OutputName](const FPFDialogueGraphTransition& Item)
			{
				return Item.OutputName == OutputName;
			})
		: nullptr;
	if (!Transition || !Transition->TargetNodeID.IsValid())
	{
		SetFailed();
		return false;
	}

	m_PendingExternalRequest = FPFDialogueExternalSelectionRequest();
	return EnterNode(Transition->TargetNodeID);
}

bool UPFDialogueGraphExecutor::BuildLinePresentation(
	const FPFDialogueRuntimeNode& Node)
{
	const FPFDialogueTableRow* Row = m_DialogueAsset->GetDialogueDataTable()
		->FindRow<FPFDialogueTableRow>(Node.RowName, TEXT("DialogueExecutor"), false);
	if (!Row || Row->RowType != EPFDialogueRowType::Dialogue)
	{
		SetFailed();
		return false;
	}

	m_CurrentLine.Speaker = Row->Speaker;
	m_CurrentLine.Text = Row->Text;
	m_ExecutionState = EPFDialogueExecutionState::WaitingForContinue;
	OnLineRequested.Broadcast(m_CurrentLine);
	return true;
}

bool UPFDialogueGraphExecutor::BuildInlineChoicePresentation(
	const FPFDialogueRuntimeNode& Node)
{
	const FPFDialogueTableRow* Row = m_DialogueAsset->GetDialogueDataTable()
		->FindRow<FPFDialogueTableRow>(Node.RowName, TEXT("DialogueExecutor"), false);
	if (!Row || Row->RowType != EPFDialogueRowType::Choice)
	{
		SetFailed();
		return false;
	}

	m_CurrentInlineChoice.Text = Row->Text;
	m_CurrentInlineChoice.Choices = GetEffectiveChoices(*Row);
	if (m_CurrentInlineChoice.Choices.IsEmpty())
	{
		SetFailed();
		return false;
	}
	m_ExecutionState = EPFDialogueExecutionState::WaitingForInlineChoice;
	OnInlineChoiceRequested.Broadcast(m_CurrentInlineChoice);
	return true;
}

bool UPFDialogueGraphExecutor::BuildExternalRequest(
	const FPFDialogueRuntimeNode& Node)
{
	const FPFDialogueTableRow* Row = m_DialogueAsset->GetDialogueDataTable()
		->FindRow<FPFDialogueTableRow>(Node.RowName, TEXT("DialogueExecutor"), false);
	if (!Row)
	{
		SetFailed();
		return false;
	}

	if (Node.NodeType == EPFDialogueGraphNodeType::ClaimSelectionRequest)
	{
		const TArray<FText> Choices = GetEffectiveChoices(*Row);
		if (Row->RowType != EPFDialogueRowType::Choice
			|| Choices.IsEmpty()
			|| Choices.Num() != Node.OptionContentIDs.Num()
			|| Node.OptionContentIDs.ContainsByPredicate(
				[](const FGameplayTag& ContentID)
				{
					return !ContentID.IsValid();
				}))
		{
			SetFailed();
			return false;
		}
	}
	else if (Row->RowType != EPFDialogueRowType::Dialogue
		|| !Node.ContextContentID.IsValid()
		|| !m_ActiveClaimID.IsValid()
		|| Node.ContextContentID != m_ActiveClaimID)
	{
		SetFailed();
		return false;
	}

	m_PendingExternalRequest = FPFDialogueExternalSelectionRequest();
	m_PendingExternalRequest.RequestID = FGuid::NewGuid();
	m_PendingExternalRequest.Prompt = Row->Text;
	m_PendingExternalRequest.CandidateContentIDs = Node.OptionContentIDs;
	m_PendingExternalRequest.ContextContentID = Node.ContextContentID;
	m_PendingExternalRequest.RequestType =
		Node.NodeType == EPFDialogueGraphNodeType::ClaimSelectionRequest
			? EPFDialogueExternalRequestType::Claim
			: EPFDialogueExternalRequestType::Evidence;
	m_ExecutionState = EPFDialogueExecutionState::WaitingForExternalResponse;
	OnExternalSelectionRequested.Broadcast(m_PendingExternalRequest);
	return true;
}

void UPFDialogueGraphExecutor::SetFailed()
{
	m_ExecutionState = EPFDialogueExecutionState::Failed;
	OnDialogueFailed.Broadcast();
}
