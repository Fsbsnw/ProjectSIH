#include "PFInvestigationSystem.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFInvestigationMessages.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/002_Systems/001_Investigation/PFClueSystem.h"

void UPFInvestigationSystem::Init()
{
	m_ActiveCaseId = FGameplayTag();

	m_ClueSystem = NewObject<UPFClueSystem>(this);
	if (!IsValid(m_ClueSystem))
	{
		PF_LOG(
			TEXT("Failed to create ClueSystem. Outer=%s"),
			*GetNameSafe(this));
		return;
	}

	m_ClueSystem->Init();
}

void UPFInvestigationSystem::Deinit()
{
	m_ActiveCaseId = FGameplayTag();

	if (IsValid(m_ClueSystem))
	{
		m_ClueSystem->Deinit();
	}

	m_ClueSystem = nullptr;
}

EPFPhaseStartResult UPFInvestigationSystem::StartInvestigation(const FPFInvestigationEntryContext& Context)
{
	if (!IsValid(m_ClueSystem))
	{
		PF_LOG(TEXT("ClueSystem is not valid"));
		return EPFPhaseStartResult::NotReady;
	}

	if (m_ActiveCaseId.IsValid())
	{
		PF_LOG(
			TEXT("Investigation is already active. ActiveCaseID=%s"),
			*m_ActiveCaseId.ToString());
		return EPFPhaseStartResult::AlreadyActive;
	}

	if (!m_ClueSystem->TryResetForCase(Context.m_CaseID))
	{
		return EPFPhaseStartResult::InvalidContext;
	}

	m_ActiveCaseId = Context.m_CaseID;
	return EPFPhaseStartResult::Started;
}

bool UPFInvestigationSystem::TryCompleteInvestigation(const FGameplayTag& CaseID)
{
	return TryFinishInvestigation(
		CaseID,
		SIHGameplayTags::Message_Flow_Investigation_Completed);
}

bool UPFInvestigationSystem::TrySkipInvestigation(const FGameplayTag& CaseID)
{
	return TryFinishInvestigation(
		CaseID,
		SIHGameplayTags::Message_Flow_Investigation_Skipped);
}

bool UPFInvestigationSystem::TryFinishInvestigation(
	const FGameplayTag& CaseID,
	const FGameplayTag& ResultChannel)
{
	if (!m_ActiveCaseId.IsValid())
	{
		PF_LOG(TEXT("There is no active Investigation to complete."));
		return false;
	}

	if (!PFGameplayTagUtilities::IsValidChildTag(
		CaseID,
		SIHGameplayTags::ID_Case))
	{
		PF_LOG(
			TEXT("CaseID is outside the Case ID domain. CaseID=%s"),
			*CaseID.ToString());
		return false;
	}

	if (!CaseID.MatchesTagExact(m_ActiveCaseId))
	{
		PF_LOG(
			TEXT(
				"Completion CaseID does not match the active Investigation. "
				"CaseID=%s, ActiveCaseID=%s"),
			*CaseID.ToString(),
			*m_ActiveCaseId.ToString());
		return false;
	}

	if (!IsValid(m_ClueSystem))
	{
		PF_LOG(TEXT("ClueSystem is not valid"));
		return false;
	}

	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		PF_LOG(TEXT("GameplayMessageSubsystem is unavailable"));
		return false;
	}

	FPFInvestigationResult Result;
	Result.m_CaseID = m_ActiveCaseId;
	if (!m_ClueSystem->TryGetAcquiredClueIDs(Result.m_AcquiredClueIDs))
	{
		PF_LOG(TEXT("Failed to read acquired Clue IDs."));
		return false;
	}

	m_ActiveCaseId = FGameplayTag();
	SendInvestigationResult(ResultChannel, Result);
	return true;
}

EPFClueAcquireResult UPFInvestigationSystem::TryAcquireClue(const FGameplayTag& ClueID)
{
	if (!m_ActiveCaseId.IsValid() || !IsValid(m_ClueSystem))
	{
		return EPFClueAcquireResult::InactiveCase;
	}

	return m_ClueSystem->TryAcquireClue(ClueID);
}

FPFClueCombinationResult UPFInvestigationSystem::TryCombineClues(
	const FGameplayTagContainer& SourceClueIDs)
{
	if (!m_ActiveCaseId.IsValid() || !IsValid(m_ClueSystem))
	{
		FPFClueCombinationResult Result;
		Result.m_Result = EPFClueCombineResult::InactiveCase;
		return Result;
	}

	return m_ClueSystem->TryCombineClues(SourceClueIDs);
}

bool UPFInvestigationSystem::HasClue(const FGameplayTag& ClueID) const
{
	return m_ActiveCaseId.IsValid()
		&& IsValid(m_ClueSystem)
		&& m_ClueSystem->HasClue(ClueID);
}

bool UPFInvestigationSystem::TryGetAcquiredClueIDs(FGameplayTagContainer& OutClueIDs) const
{
	if (!m_ActiveCaseId.IsValid() || !IsValid(m_ClueSystem))
	{
		OutClueIDs.Reset();
		return false;
	}

	return m_ClueSystem->TryGetAcquiredClueIDs(OutClueIDs);
}

void UPFInvestigationSystem::SendInvestigationResult(
	const FGameplayTag& Channel,
	const FPFInvestigationResult& Result)
{
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(Channel, Result);
}
