#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "PFCharacterStateTypes.generated.h"

USTRUCT(BlueprintType)
struct FPFCharacterProgressData
{
	GENERATED_BODY()

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Character Progress",
		meta = (DisplayName = "Level"))
	int32 m_Level = 1;

	bool IsValid() const
	{
		return m_Level >= 1;
	}
};

USTRUCT(BlueprintType)
struct FPFEquipmentLoadout
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FGameplayTag, FGuid> m_EquippedItemInstanceIDs;

	bool IsValid() const
	{
		for (const TPair<FGameplayTag, FGuid>& EquippedItem :
			m_EquippedItemInstanceIDs)
		{
			const FGameplayTag& SlotTag = EquippedItem.Key;
			const FGuid& ItemInstanceID = EquippedItem.Value;

			const bool bIsEquipmentSlot =
				SlotTag.MatchesTag(SIHGameplayTags::Equipment_Slot)
				&& SlotTag != SIHGameplayTags::Equipment_Slot;

			if (!bIsEquipmentSlot || !ItemInstanceID.IsValid())
			{
				return false;
			}
		}

		return true;
	}
};

USTRUCT(BlueprintType)
struct FPFSkillProgressData
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FPFCharacterStateData
{
	GENERATED_BODY()

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Character State",
		meta = (DisplayName = "Progress Data"))
	FPFCharacterProgressData m_ProgressData;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Character State",
		meta = (DisplayName = "Equipment Loadout"))
	FPFEquipmentLoadout m_EquipmentLoadout;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Character State",
		meta = (DisplayName = "Skill Progress Data"))
	FPFSkillProgressData m_SkillProgressData;

	bool IsValid() const
	{
		return m_ProgressData.IsValid()
			&& m_EquipmentLoadout.IsValid();
	}
};
