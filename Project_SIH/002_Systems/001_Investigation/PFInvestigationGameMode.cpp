#include "PFInvestigationGameMode.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFInvestigationMessages.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/001_Investigation/PFInvestigationSystem.h"
#include "Project_SIH/002_Systems/001_Investigation/PFMeetingSystem.h"
#include "Project_SIH/002_Systems/007_Player/PFPlayerController.h"

APFInvestigationGameMode::APFInvestigationGameMode()
{
	PlayerControllerClass = APFPlayerController::StaticClass();
}

EPFPhaseStartResult APFInvestigationGameMode::StartMeeting(const FPFMeetingEntryContext& Context)
{
	if (!Context.IsValid())
	{
		PF_LOG(TEXT("Meeting context is invalid"));
		return EPFPhaseStartResult::InvalidContext;
	}

	if (!IsValid(m_MeetingSystem))
	{
		PF_LOG(TEXT("MeetingSystem is not ready"));
		return EPFPhaseStartResult::NotReady;
	}

	return m_MeetingSystem->StartMeeting(Context);
}

EPFPhaseStartResult APFInvestigationGameMode::StartInvestigation(const FPFInvestigationEntryContext& Context)
{
	if (!Context.IsValid())
	{
		PF_LOG(TEXT("Investigation context has an invalid CaseID"));
		return EPFPhaseStartResult::InvalidContext;
	}

	if (!IsValid(m_InvestigationSystem))
	{
		PF_LOG(TEXT("InvestigationSystem is not ready"));
		return EPFPhaseStartResult::NotReady;
	}

	return m_InvestigationSystem->StartInvestigation(Context);
}

EPFClueAcquireResult APFInvestigationGameMode::TryAcquireClue(const FGameplayTag& ClueID)
{
	if (!IsValid(m_InvestigationSystem))
	{
		return EPFClueAcquireResult::InactiveCase;
	}

	return m_InvestigationSystem->TryAcquireClue(ClueID);
}

FPFClueCombinationResult APFInvestigationGameMode::TryCombineClues(
	const FGameplayTagContainer& SourceClueIDs)
{
	if (!IsValid(m_InvestigationSystem))
	{
		FPFClueCombinationResult Result;
		Result.m_Result = EPFClueCombineResult::InactiveCase;
		return Result;
	}

	return m_InvestigationSystem->TryCombineClues(SourceClueIDs);
}

bool APFInvestigationGameMode::TryCompleteInvestigation(const FGameplayTag& CaseID)
{
	if (!IsValid(m_InvestigationSystem))
	{
		return false;
	}

	return m_InvestigationSystem->TryCompleteInvestigation(CaseID);
}

bool APFInvestigationGameMode::TrySkipInvestigation(const FGameplayTag& CaseID)
{
	return IsValid(m_InvestigationSystem)
		&& m_InvestigationSystem->TrySkipInvestigation(CaseID);
}

bool APFInvestigationGameMode::TryGetAcquiredClueIDs(
	FGameplayTagContainer& OutClueIDs) const
{
	OutClueIDs.Reset();

	return IsValid(m_MeetingSystem)
		&& m_MeetingSystem->TryGetAcquiredClueIDs(OutClueIDs);
}

EPFMeetingClaimSelectResult APFInvestigationGameMode::TrySelectClaim(const FGameplayTag& ClaimID)
{
	if (!IsValid(m_MeetingSystem))
	{
		return EPFMeetingClaimSelectResult::InactiveMeeting;
	}

	return m_MeetingSystem->TrySelectClaim(ClaimID);
}

EPFMeetingSubmitResult APFInvestigationGameMode::TrySubmitClue(const FGameplayTag& ClueID)
{
	if (!IsValid(m_MeetingSystem))
	{
		return EPFMeetingSubmitResult::InactiveMeeting;
	}

	return m_MeetingSystem->TrySubmitClue(ClueID);
}

bool APFInvestigationGameMode::TryCompleteMeeting(const FGameplayTag& CaseID)
{
	return IsValid(m_MeetingSystem)
		&& m_MeetingSystem->TryCompleteMeeting(CaseID);
}

void APFInvestigationGameMode::InitGame(
	const FString& MapName,
	const FString& Options,
	FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// PF_LOG(
	// 	TEXT("Initializing world systems. GameMode=%s, World=%s"),
	// 	*GetNameSafe(this),
	// 	*GetNameSafe(GetWorld()));

	m_InvestigationSystem = NewObject<UPFInvestigationSystem>(this);
	if (IsValid(m_InvestigationSystem))
	{
		m_InvestigationSystem->Init();
	}
	else
	{
		PF_LOG(
			TEXT("Failed to create InvestigationSystem. Outer=%s"),
			*GetNameSafe(this));
	}

	m_MeetingSystem = NewObject<UPFMeetingSystem>(this);
	if (IsValid(m_MeetingSystem))
	{
		m_MeetingSystem->Init();
	}
	else
	{
		PF_LOG(
			TEXT("Failed to create MeetingSystem. Outer=%s"),
			*GetNameSafe(this));
	}

	// PF_LOG(
	// 	TEXT("World system initialization complete. "
	// 		 "InvestigationSystem=%s, MeetingSystem=%s"),
	// 	*GetNameSafe(m_InvestigationSystem),
	// 	*GetNameSafe(m_MeetingSystem));


}

void APFInvestigationGameMode::BeginPlay()
{
	Super::BeginPlay();

	SendInvestigationReadyMessage();
}

void APFInvestigationGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(m_InvestigationSystem))
	{
		m_InvestigationSystem->Deinit();
	}

	if (IsValid(m_MeetingSystem))
	{
		m_MeetingSystem->Deinit();
	}

	Super::EndPlay(EndPlayReason);
}


void APFInvestigationGameMode::SendInvestigationReadyMessage()
{
	if (!IsValid(m_InvestigationSystem) || !IsValid(m_MeetingSystem))
	{
		PF_LOG(TEXT("Investigation systems are not ready"));
		return;
	}

	UGameplayMessageSubsystem& GameMessageSubsystem =
		UGameplayMessageSubsystem::Get(this);

	GameMessageSubsystem.BroadcastMessage(
		SIHGameplayTags::Message_Flow_Investigation_Ready,
		FPFInvestigationReadyMessage());
}
