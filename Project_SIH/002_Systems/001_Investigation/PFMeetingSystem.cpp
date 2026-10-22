#include "PFMeetingSystem.h"

#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFMeetingMessages.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/001_Data/000_Definitions/PFCaseDefinition.h"
#include "Project_SIH/001_Data/000_Definitions/PFClueDefinition.h"
#include "Project_SIH/001_Data/000_Definitions/PFDialogueDefinition.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"
#include "Project_SIH/002_Systems/011_Dialogue/PFDialogueSubsystem.h"
#include "Project_SIH/003_UI/002_Subsystem/PFUIManagerSubsystem.h"

namespace
{
	const FName MeetingResult_Correct(TEXT("Correct"));
	const FName MeetingResult_Incorrect(TEXT("Incorrect"));
}

void UPFMeetingSystem::Init()
{
	ResetState();
}

void UPFMeetingSystem::Deinit()
{
	UnbindDialogueEvents();
	if (IsValid(m_DialogueSubsystem)
		&& m_DialogueSubsystem->IsDialogueActive())
	{
		m_DialogueSubsystem->CancelDialogue();
	}
	m_DialogueSubsystem = nullptr;
	ResetState();
}

EPFPhaseStartResult UPFMeetingSystem::StartMeeting(const FPFMeetingEntryContext& Context)
{
	if (!Context.IsValid())
	{
		PF_LOG(TEXT("Context is invalid"));
		return EPFPhaseStartResult::InvalidContext;
	}

	if (m_CaseID.IsValid())
	{
		PF_LOG(TEXT("Meeting is already active. CaseID=%s"), *m_CaseID.ToString());
		return EPFPhaseStartResult::AlreadyActive;
	}

	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		PF_LOG(TEXT("GameplayMessageSubsystem is unavailable"));
		return EPFPhaseStartResult::NotReady;
	}

	// Meeting은 대화 시스템이 선택한 Claim을 판정할 뿐,
	// 획득 단서로 표시할 Claim 목록을 구성하지 않습니다.
	for (const FGameplayTag& AcquiredClueID : Context.m_AcquiredClueIDs)
	{
		if (!PFGameplayTagUtilities::IsValidChildTag(
			AcquiredClueID,
			SIHGameplayTags::ID_Clue))
		{
			PF_LOG(
				TEXT("Meeting context contains an invalid ClueID. ClueID=%s"),
				*AcquiredClueID.ToString());
			return EPFPhaseStartResult::InvalidContext;
		}
	}

	m_CaseID = Context.m_CaseID;
	m_AcquiredClueIDs = Context.m_AcquiredClueIDs;
	m_RevealedWeaknessIDs.Reset();
	m_SelectedClaimID = FGameplayTag();
	m_InputStep = EPFMeetingInputStep::SelectingClaim;

	if (!TryStartMeetingDialogue())
	{
		ResetState();
		return EPFPhaseStartResult::NotReady;
	}

	return EPFPhaseStartResult::Started;
}

bool UPFMeetingSystem::TryStartMeetingDialogue()
{
	const UPFCaseDefinition* CaseDefinition =
		UPFAssetManager::Get().GetCaseDefinition(m_CaseID);
	if (!IsValid(CaseDefinition))
	{
		PF_LOG(TEXT("Active Case Definition is unavailable"));
		return false;
	}

	const FGameplayTag& DialogueID =
		CaseDefinition->GetMeetingDialogueID();
	if (!DialogueID.IsValid())
	{
		PF_LOG(
			TEXT("Meeting Dialogue ID is not configured. CaseID=%s"),
			*m_CaseID.ToString());
		return false;
	}

	UPFDialogueDefinition* DialogueDefinition =
		UPFAssetManager::Get().GetDialogueDefinition(DialogueID);
	if (!IsValid(DialogueDefinition))
	{
		PF_LOG(
			TEXT("Meeting Dialogue Definition is unavailable. DialogueID=%s"),
			*DialogueID.ToString());
		return false;
	}

	UWorld* World = GetWorld();
	m_DialogueSubsystem = IsValid(World)
		? World->GetSubsystem<UPFDialogueSubsystem>()
		: nullptr;
	if (!IsValid(m_DialogueSubsystem))
	{
		PF_LOG(TEXT("DialogueSubsystem is not available"));
		return false;
	}

	if (m_DialogueSubsystem->IsDialogueActive())
	{
		PF_LOG(TEXT("Another Dialogue is already active"));
		m_DialogueSubsystem = nullptr;
		return false;
	}

	UnbindDialogueEvents();
	BindDialogueEvents();
	const EPFDialogueInputResult DialogueResult =
		m_DialogueSubsystem->StartDialogue(DialogueDefinition);
	if (DialogueResult != EPFDialogueInputResult::Accepted
		|| !m_DialogueSubsystem->IsDialogueActive())
	{
		PF_LOG(
			TEXT("Meeting Dialogue start rejected. Result=%d, DialogueID=%s"),
			static_cast<uint8>(DialogueResult),
			*DialogueID.ToString());
		UnbindDialogueEvents();
		m_DialogueSubsystem = nullptr;
		return false;
	}

	return true;
}

void UPFMeetingSystem::BindDialogueEvents()
{
	if (!IsValid(m_DialogueSubsystem))
	{
		return;
	}

	m_DialogueSubsystem->GetCompletedDelegate().AddUniqueDynamic(
		this,
		&UPFMeetingSystem::HandleDialogueCompleted);
	m_DialogueSubsystem->GetActionRequestedDelegate().AddUniqueDynamic(
		this,
		&UPFMeetingSystem::HandleDialogueActionRequested);
}

void UPFMeetingSystem::UnbindDialogueEvents()
{
	if (!IsValid(m_DialogueSubsystem))
	{
		return;
	}

	m_DialogueSubsystem->GetCompletedDelegate().RemoveDynamic(
		this,
		&UPFMeetingSystem::HandleDialogueCompleted);
	m_DialogueSubsystem->GetActionRequestedDelegate().RemoveDynamic(
		this,
		&UPFMeetingSystem::HandleDialogueActionRequested);
}

void UPFMeetingSystem::HandleDialogueActionRequested(const FPFDialogueActionPresentation& Presentation)
{
	if (!Presentation.m_Action.m_ActionID.MatchesTagExact(SIHGameplayTags::Action_Dialogue_Submit.GetTag()))
	{
		return;
	}

	m_PendingDialogueActionNodeID = Presentation.m_NodeID;
	m_SelectedClaimID = FGameplayTag();
	m_InputStep = EPFMeetingInputStep::SelectingClaim;

	if (!TryShowClueSubmissionWidget())
	{
		PF_LOG(TEXT("Meeting submission screen could not be shown"));
	}
}

bool UPFMeetingSystem::TryShowClueSubmissionWidget() const
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = IsValid(World) ? World->GetGameInstance() : nullptr;
	ULocalPlayer* LocalPlayer = IsValid(GameInstance) ? GameInstance->GetFirstGamePlayer() : nullptr;
	UPFUIManagerSubsystem* UIManager = IsValid(LocalPlayer) ? LocalPlayer->GetSubsystem<UPFUIManagerSubsystem>() : nullptr;
	return IsValid(UIManager) && UIManager->ShowWidget(SIHGameplayTags::UI_Screen_ClueSubmission.GetTag()) != nullptr;
}

void UPFMeetingSystem::HandleDialogueCompleted()
{
	if (!m_CaseID.IsValid()
		|| !UGameplayMessageSubsystem::HasInstance(this))
	{
		PF_LOG(TEXT("Meeting cannot complete after Dialogue completion"));
		return;
	}

	CompleteMeeting();
}

bool UPFMeetingSystem::TryGetAcquiredClueIDs(
	FGameplayTagContainer& OutClueIDs) const
{
	OutClueIDs.Reset();

	if (!m_CaseID.IsValid())
	{
		return false;
	}

	// 회의 단계에서 사용할 수 없는 단서들을 제외합니다.
	FGameplayTagContainer MeetingUsableClueIDs;
	for (const FGameplayTag& AcquiredClueID : m_AcquiredClueIDs)
	{
		const UPFClueDefinition* ClueDefinition =
			UPFAssetManager::Get().GetClueDefinition(AcquiredClueID);
		if (!ClueDefinition)
		{
			PF_LOG(
				TEXT("Acquired Clue definition is unavailable. ClueID=%s"),
				*AcquiredClueID.ToString());
			return false;
		}

		if (ClueDefinition->IsMeetingUsable())
		{
			MeetingUsableClueIDs.AddTag(AcquiredClueID);
		}
	}

	OutClueIDs = MoveTemp(MeetingUsableClueIDs);
	return true;
}

EPFMeetingClaimSelectResult UPFMeetingSystem::TrySelectClaim(const FGameplayTag& ClaimID)
{
	if (!m_CaseID.IsValid())
	{
		PF_LOG(TEXT("There is no active Meeting to select a Claim."));
		return EPFMeetingClaimSelectResult::InactiveMeeting;
	}

	if (m_InputStep != EPFMeetingInputStep::SelectingClaim)
	{
		PF_LOG(TEXT("Meeting is not waiting for a Claim selection."));
		return EPFMeetingClaimSelectResult::InvalidStep;
	}

	if (!PFGameplayTagUtilities::IsValidChildTag(ClaimID, SIHGameplayTags::ID_Claim))
	{
		PF_LOG(TEXT("ClaimID is outside the Claim ID domain. ClaimID=%s"), *ClaimID.ToString());
		return EPFMeetingClaimSelectResult::InvalidClaimID;
	}

	m_SelectedClaimID = ClaimID;
	m_InputStep = EPFMeetingInputStep::SelectingClue;
	return EPFMeetingClaimSelectResult::Selected;
}

EPFMeetingSubmitResult UPFMeetingSystem::TrySubmitClue(const FGameplayTag& ClueID)
{
	if (!m_CaseID.IsValid())
	{
		PF_LOG(TEXT("There is no active Meeting to submit a Clue."));
		return EPFMeetingSubmitResult::InactiveMeeting;
	}

	if (m_InputStep != EPFMeetingInputStep::SelectingClue
		|| !m_SelectedClaimID.IsValid())
	{
		PF_LOG(TEXT("Meeting is not waiting for a Clue submission."));
		return EPFMeetingSubmitResult::InvalidStep;
	}

	if (!PFGameplayTagUtilities::IsValidChildTag(ClueID, SIHGameplayTags::ID_Clue))
	{
		PF_LOG(TEXT("ClueID is outside the Clue ID domain. ClueID=%s"), *ClueID.ToString());
		return EPFMeetingSubmitResult::InvalidClueID;
	}

	if (!m_AcquiredClueIDs.HasTagExact(ClueID))
	{
		PF_LOG(TEXT("Clue is not available for submission. ClueID=%s"), *ClueID.ToString());
		return EPFMeetingSubmitResult::ClueNotOwned;
	}

	const UPFClueDefinition* ClueDefinition =
		UPFAssetManager::Get().GetClueDefinition(ClueID);
	if (!ClueDefinition)
	{
		PF_LOG(TEXT("Clue definition is unavailable. ClueID=%s"), *ClueID.ToString());
		return EPFMeetingSubmitResult::NotReady;
	}

	if (!ClueDefinition->IsMeetingUsable())
	{
		PF_LOG(TEXT("Clue is not usable in Meeting. ClueID=%s"), *ClueID.ToString());
		return EPFMeetingSubmitResult::ClueNotMeetingUsable;
	}

	if (!PFGameplayTagUtilities::IsValidChildTag(
			ClueDefinition->GetClaimID(),
			SIHGameplayTags::ID_Claim)
		|| !PFGameplayTagUtilities::IsValidChildTag(
			ClueDefinition->GetRevealedWeaknessID(),
			SIHGameplayTags::ID_Element))
	{
		PF_LOG(TEXT("Clue Meeting definition is invalid. ClueID=%s"), *ClueID.ToString());
		return EPFMeetingSubmitResult::NotReady;
	}

	const bool bIsCorrect =	m_SelectedClaimID == ClueDefinition->GetClaimID();
	if (bIsCorrect)
	{
		m_RevealedWeaknessIDs.Add(
			ClueDefinition->GetRevealedWeaknessID());
	}


	// 다음 선택 지점과 Meeting 종료 시점은 대화 시스템이 결정합니다.
	m_SelectedClaimID = FGameplayTag();
	m_InputStep = EPFMeetingInputStep::SelectingClaim;

	const EPFMeetingSubmitResult SubmitResult = bIsCorrect
		? EPFMeetingSubmitResult::Correct
		: EPFMeetingSubmitResult::Incorrect;
	CompletePendingDialogueAction(SubmitResult);
	return SubmitResult;
}

void UPFMeetingSystem::CompletePendingDialogueAction(const EPFMeetingSubmitResult SubmitResult)
{
	if (!IsValid(m_DialogueSubsystem) || m_PendingDialogueActionNodeID.IsNone())
	{
		return;
	}

	const FName ResultID = SubmitResult == EPFMeetingSubmitResult::Correct
		? MeetingResult_Correct
		: MeetingResult_Incorrect;
	const FName ActionNodeID = m_PendingDialogueActionNodeID;
	m_PendingDialogueActionNodeID = NAME_None;

	const EPFDialogueInputResult DialogueResult = m_DialogueSubsystem->CompleteAction(ActionNodeID, ResultID);
	if (DialogueResult != EPFDialogueInputResult::Accepted)
	{
		PF_LOG(TEXT("Meeting Dialogue Action completion was rejected. NodeID=%s, ResultID=%s, Result=%d"), *ActionNodeID.ToString(), *ResultID.ToString(), static_cast<uint8>(DialogueResult));
	}
}

bool UPFMeetingSystem::TryCompleteMeeting(const FGameplayTag& CaseID)
{
	if (!m_CaseID.IsValid() || m_CaseID != CaseID)
	{
		return false;
	}

	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return false;
	}

	UnbindDialogueEvents();
	if (IsValid(m_DialogueSubsystem)
		&& m_DialogueSubsystem->IsDialogueActive())
	{
		m_DialogueSubsystem->CancelDialogue();
	}
	CompleteMeeting();
	return true;
}

void UPFMeetingSystem::CompleteMeeting()
{
	FPFMeetingResult Result{};
	Result.m_CaseID = m_CaseID;
	Result.m_RevealedWeaknessIDs = m_RevealedWeaknessIDs;

	UnbindDialogueEvents();
	m_DialogueSubsystem = nullptr;
	ResetState();
	SendMeetingCompletedMessage(Result);
}

void UPFMeetingSystem::ResetState()
{
	m_CaseID = FGameplayTag();
	m_AcquiredClueIDs.Reset();
	m_RevealedWeaknessIDs.Reset();
	m_SelectedClaimID = FGameplayTag();
	m_PendingDialogueActionNodeID = NAME_None;
	m_InputStep = EPFMeetingInputStep::Inactive;
}

void UPFMeetingSystem::SendMeetingCompletedMessage(const FPFMeetingResult& Result)
{
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		SIHGameplayTags::Message_Flow_Meeting_Completed,
		Result);
}
