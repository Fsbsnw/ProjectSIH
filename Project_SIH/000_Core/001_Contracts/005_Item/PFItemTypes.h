#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/004_Character/PFCharacterStatTypes.h"
#include "PFItemTypes.generated.h"

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFItemFragment
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFEquipmentFragment : public FPFItemFragment
{
	GENERATED_BODY()

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Equipment",
		meta = (
			DisplayName = "Slot Tag",
			Categories = "Equipment.Slot",
			ToolTip = "이 장비가 사용하는 장착 슬롯입니다"))
	FGameplayTag m_SlotTag;
};

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFEquipmentStatModifierFragment
	: public FPFItemFragment
{
	GENERATED_BODY()

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Equipment|Stat",
		meta = (
			DisplayName = "Stat Modifiers"))
	FPFCharacterStatModifiers m_StatModifiers;
};
