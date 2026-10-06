#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFCombatProfileTypes.h"
#include "PFCaseDefinition.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFBossEncounterDefinition
{
	GENERATED_BODY()

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Boss Encounter",
		meta = (
			DisplayName = "Character ID",
			Categories = "ID.Character"))
	FGameplayTag m_CharacterID;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Boss Encounter",
		meta = (DisplayName = "Combat Profile"))
	FPFCombatProfile m_CombatProfile;

	// 고정된 네 슬롯의 원본 순서다. 같은 속성을 여러 슬롯에 둘 수 있다.
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Boss Encounter",
		meta = (DisplayName = "Weakness Elements", Categories = "ID.Element"))
	TArray<FGameplayTag> m_WeaknessElementIDs;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Boss Encounter",
		meta = (DisplayName = "Weakness Debuff Effect"))
	TSoftClassPtr<UGameplayEffect> m_WeaknessDebuffEffect;
};

UCLASS()
class PROJECT_SIH_API UPFCaseDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	static FPrimaryAssetId MakePrimaryAssetID(const FGameplayTag& CaseID);

	const FGameplayTag& GetCaseID() const
	{
		return m_CaseID;
	}

	const FPFBossEncounterDefinition& GetBossEncounter() const
	{
		return m_BossEncounter;
	}

private:
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Definition",
		meta = (AllowPrivateAccess = "true",
			DisplayName = "Case ID",
			Categories = "ID.Case",
			ToolTip = "Case를 식별하는 고유 GameplayTag입니다. ID.Case.*의 형태입니다"))
	FGameplayTag m_CaseID;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Definition|Battle",
		meta = (DisplayName = "Boss Encounter"))
	FPFBossEncounterDefinition m_BossEncounter;
};
