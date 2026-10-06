#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/004_Character/PFCharacterStatTypes.h"

class APFBattleCharacterBase;
class UCurveTable;
class UPFAbilityBase;

struct FPFCombatProfile;

struct FPFResolvedCombatProfile
{
	TSubclassOf<APFBattleCharacterBase>
		m_CharacterClassToSpawn;

	FPFCharacterStatValues m_BaseStats;
	FGameplayTag m_ElementID;

	TArray<TSubclassOf<UPFAbilityBase>>
		m_InitialAbilities;

	const UCurveTable* m_LevelBonusTable = nullptr;
};

class PROJECT_SIH_API FPFCombatProfileResolver
{
public:
	bool ResolveAllyProfile(
		const FGameplayTag& CharacterID,
		FPFResolvedCombatProfile& OutProfile) const;

	bool ResolveBossProfile(
		const FGameplayTag& CaseID,
		FPFResolvedCombatProfile& OutProfile) const;

private:
	bool ResolveProfile(
		const FPFCombatProfile& CombatProfile,
		const UCurveTable* LevelBonusTable,
		FPFResolvedCombatProfile& OutProfile) const;

	bool LoadInitialAbilities(
		const TSet<TSoftClassPtr<UPFAbilityBase>>&
			InitialAbilities,
		TArray<TSubclassOf<UPFAbilityBase>>&
			OutAbilityClasses) const;
};
