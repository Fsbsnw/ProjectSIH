#pragma once
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFInvestigationMessages.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFMeetingMessages.h"
#include "Project_SIH/000_Core/001_Contracts/002_Party/PFPartyTypes.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleMessages.h"


enum class EPFCasePhase : uint8
{
	None,
	Investigation,
	Meeting,
	PartyFormation,
	Battle,
	Result,
};

enum class EPFPhaseStartResult : uint8
{
	None,
	Started,
	AlreadyActive,
	InvalidContext,
	NotReady,
};

struct FPFActiveCaseState
{
	FGameplayTag m_CaseID;
	EPFCasePhase m_CasePhase = EPFCasePhase::None;
	FPFInvestigationResult m_InvestigationResult;
	FPFMeetingResult m_MeetingResult;
	FPFPartyData m_CurrentParty;
	FPFBattleResult m_BattleResult;

	bool IsActiveInPhase(
		const EPFCasePhase CasePhase) const
	{
		return m_CaseID.IsValid()
			&& m_CasePhase == CasePhase;
	}

	bool MatchesCaseAndPhase(
		const FGameplayTag& CaseID,
		const EPFCasePhase CasePhase) const
	{
		return CaseID.IsValid()
			&& m_CaseID == CaseID
			&& IsActiveInPhase(CasePhase);
	}
};

struct FPFInvestigationEntryContext
{
	FGameplayTag m_CaseID;

	bool IsValid() const
	{
		return m_CaseID.IsValid();
	}
};

struct FPFMeetingEntryContext
{
	FGameplayTag m_CaseID;
	FGameplayTagContainer m_AcquiredClueIDs;

	bool IsValid() const
	{
		return m_CaseID.IsValid();
	}
};

struct FPFBattleEntryContext
{
	FGameplayTag m_CaseID;
	FPFPartyData m_Party;
	TArray<FGameplayTag> m_RevealedWeaknessIDs;

	bool IsValid() const
	{
		return m_CaseID.IsValid() && m_Party.IsValid();
	}
};
