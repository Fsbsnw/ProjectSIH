#include "PFCombatProfileResolver.h"

#include "Engine/CurveTable.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFCombatProfileTypes.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/001_Data/000_Definitions/PFCaseDefinition.h"
#include "Project_SIH/001_Data/000_Definitions/PFCharacterDefinition.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"
#include "Project_SIH/002_Systems/003_Combat/001_Characters/PFBattleCharacterBase.h"
#include "Project_SIH/002_Systems/009_AbilitySystem/PFAbilityBase.h"

bool FPFCombatProfileResolver::ResolveAllyProfile(
	const FGameplayTag& CharacterID,
	FPFResolvedCombatProfile& OutProfile) const
{
	OutProfile = FPFResolvedCombatProfile();

	UPFAssetManager& AssetManager =
		UPFAssetManager::Get();

	const UPFCharacterDefinition* CharacterDefinition =
		AssetManager.GetCharacterDefinition(CharacterID);

	if (!IsValid(CharacterDefinition))
	{
		PF_LOG(
			TEXT(
				"CharacterDefinition is unavailable. "
				"CharacterID=%s"),
			*CharacterID.ToString());
		return false;
	}

	const UCurveTable* LevelBonusTable =
		CharacterDefinition->GetLevelBonusTable();

	if (!IsValid(LevelBonusTable))
	{
		PF_LOG(
			TEXT(
				"LevelBonusTable is unavailable. "
				"CharacterID=%s"),
			*CharacterID.ToString());
		return false;
	}

	return ResolveProfile(
		CharacterDefinition->GetDefaultCombatProfile(),
		LevelBonusTable,
		OutProfile);
}

bool FPFCombatProfileResolver::ResolveBossProfile(
	const FGameplayTag& CaseID,
	FPFResolvedCombatProfile& OutProfile) const
{
	OutProfile = FPFResolvedCombatProfile();

	UPFAssetManager& AssetManager =
		UPFAssetManager::Get();

	const UPFCaseDefinition* CaseDefinition =
		AssetManager.GetCaseDefinition(CaseID);

	if (!IsValid(CaseDefinition))
	{
		PF_LOG(
			TEXT(
				"CaseDefinition is unavailable. "
				"CaseID=%s"),
			*CaseID.ToString());
		return false;
	}

	const FPFBossEncounterDefinition& BossEncounter =
		CaseDefinition->GetBossEncounter();

	if (!IsValid(
		AssetManager.GetCharacterDefinition(
			BossEncounter.m_CharacterID)))
	{
		PF_LOG(
			TEXT(
				"Boss CharacterDefinition is unavailable. "
				"CharacterID=%s"),
			*BossEncounter.m_CharacterID.ToString());
		return false;
	}

	return ResolveProfile(
		BossEncounter.m_CombatProfile,
		nullptr,
		OutProfile);
}

bool FPFCombatProfileResolver::ResolveProfile(
	const FPFCombatProfile& CombatProfile,
	const UCurveTable* LevelBonusTable,
	FPFResolvedCombatProfile& OutProfile) const
{
	OutProfile = FPFResolvedCombatProfile();

	UClass* CharacterClassToSpawn =
		CombatProfile
			.m_CharacterClassToSpawn
			.LoadSynchronous();

	if (!IsValid(CharacterClassToSpawn))
	{
		PF_LOG(
			TEXT(
				"Character class to spawn is invalid. "
				"Class=%s"),
			*GetNameSafe(CharacterClassToSpawn));
		return false;
	}

	TArray<TSubclassOf<UPFAbilityBase>>
		LoadedAbilityClasses;

	if (!LoadInitialAbilities(
		CombatProfile.m_InitialAbilities,
		LoadedAbilityClasses))
	{
		return false;
	}

	OutProfile.m_CharacterClassToSpawn =
		CharacterClassToSpawn;

	OutProfile.m_BaseStats =
		CombatProfile.m_BaseStats;

	OutProfile.m_ElementID =
		CombatProfile.m_ElementID;

	OutProfile.m_InitialAbilities =
		MoveTemp(LoadedAbilityClasses);

	OutProfile.m_LevelBonusTable =
		LevelBonusTable;

	return true;
}

bool FPFCombatProfileResolver::LoadInitialAbilities(
	const TSet<TSoftClassPtr<UPFAbilityBase>>&
		InitialAbilities,
	TArray<TSubclassOf<UPFAbilityBase>>&
		OutAbilityClasses) const
{
	OutAbilityClasses.Reset();
	OutAbilityClasses.Reserve(InitialAbilities.Num());

	for (const TSoftClassPtr<UPFAbilityBase>&
		SoftAbilityClass : InitialAbilities)
	{
		UClass* AbilityClass =
			SoftAbilityClass.LoadSynchronous();

		if (!IsValid(AbilityClass))
		{
			PF_LOG(
				TEXT(
					"Initial AbilityClass is invalid. "
					"Path=%s"),
				*SoftAbilityClass
					.ToSoftObjectPath()
					.ToString());
			return false;
		}

		OutAbilityClasses.Add(AbilityClass);
	}

	return true;
}
