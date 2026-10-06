#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/004_Character/PFCharacterStatTypes.h"
#include "PFCombatProfileTypes.generated.h"

class APFBattleCharacterBase;
class UPFAbilityBase;

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFCombatProfile
{
	GENERATED_BODY()

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Combat Profile",
		meta = (DisplayName = "Character Class To Spawn"))
	TSoftClassPtr<APFBattleCharacterBase>
		m_CharacterClassToSpawn;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Combat Profile",
		meta = (DisplayName = "Base Stats"))
	FPFCharacterStatValues m_BaseStats;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Combat Profile",
		meta = (DisplayName = "Initial Abilities"))
	TSet<TSoftClassPtr<UPFAbilityBase>> m_InitialAbilities;

	// 파티원 한 명의 속성. Boss CombatProfile에서는 사용하지 않는다.
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Combat Profile",
		meta = (DisplayName = "Element ID", Categories = "ID.Element"))
	FGameplayTag m_ElementID;
};
