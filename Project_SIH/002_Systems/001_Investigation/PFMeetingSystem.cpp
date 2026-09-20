#include "PFMeetingSystem.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFMeetingMessages.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/001_Data/000_Definitions/PFClueDefinition.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"

void UPFMeetingSystem::Init()
{
	ResetState();
}

void UPFMeetingSystem::Deinit()
{
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

	return EPFPhaseStartResult::Started;
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
			SIHGameplayTags::ID_Weakness))
	{
		PF_LOG(TEXT("Clue Meeting definition is invalid. ClueID=%s"), *ClueID.ToString());
		return EPFMeetingSubmitResult::NotReady;
	}

	const bool bIsCorrect =	m_SelectedClaimID == ClueDefinition->GetClaimID();
	if (bIsCorrect)
	{
		m_RevealedWeaknessIDs.AddTag(
			ClueDefinition->GetRevealedWeaknessID());
	}


	// 다음 선택 지점과 Meeting 종료 시점은 대화 시스템이 결정합니다.
	m_SelectedClaimID = FGameplayTag();
	m_InputStep = EPFMeetingInputStep::SelectingClaim;

	return bIsCorrect
		? EPFMeetingSubmitResult::Correct
		: EPFMeetingSubmitResult::Incorrect;
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

	FPFMeetingResult Result{};
	Result.m_CaseID = m_CaseID;
	Result.m_RevealedWeaknessIDs = m_RevealedWeaknessIDs;

	ResetState();
	SendMeetingCompletedMessage(Result);
	return true;
}

void UPFMeetingSystem::ResetState()
{
	m_CaseID = FGameplayTag();
	m_AcquiredClueIDs.Reset();
	m_RevealedWeaknessIDs.Reset();
	m_SelectedClaimID = FGameplayTag();
	m_InputStep = EPFMeetingInputStep::Inactive;
}

void UPFMeetingSystem::SendMeetingCompletedMessage(const FPFMeetingResult& Result)
{
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		SIHGameplayTags::Message_Flow_Meeting_Completed,
		Result);
}
