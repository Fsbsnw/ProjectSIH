#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleTypes.h"
#include "PFBattleMessages.generated.h"

class APFBattleCharacterBase;

USTRUCT()
struct PROJECT_SIH_API FPFBattleReadyMessage
{
	GENERATED_BODY()
};

USTRUCT()
struct PROJECT_SIH_API FPFBattleParticipantEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<APFBattleCharacterBase> m_Character;

	UPROPERTY()
	FGameplayTag m_CharacterID;
};

USTRUCT()
struct PROJECT_SIH_API FPFBattleParticipantsInitializedMessage
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPFBattleParticipantEntry> m_Allies;

	UPROPERTY()
	FPFBattleParticipantEntry m_Boss;
};

USTRUCT()
struct PROJECT_SIH_API FPFBattleWeaknessSlotsMessage
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FGameplayTag> m_VisibleElementIDs;
};

USTRUCT()
struct PROJECT_SIH_API FPFBattleActionStateMessage
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<APFBattleCharacterBase> m_ActionOwner;

	UPROPERTY()
	bool m_bCanPlayerSelectAction = false;

	UPROPERTY()
	FGameplayTag m_SelectedActionTag;

	UPROPERTY()
	FPFBattleTargetSelection m_TargetSelection;
};

USTRUCT()
struct PROJECT_SIH_API FPFBattleResult
{
	GENERATED_BODY()

	FGameplayTag m_CaseID;
	EPFBattleResult m_ResultType = EPFBattleResult::None;

	bool IsValid() const
	{
		return m_CaseID.IsValid() &&
			(m_ResultType == EPFBattleResult::NormalVictory ||
				m_ResultType == EPFBattleResult::ForcedEviction ||
				m_ResultType == EPFBattleResult::Defeat);
	}
};
