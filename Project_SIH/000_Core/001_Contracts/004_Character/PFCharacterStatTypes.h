#pragma once

#include "CoreMinimal.h"
#include "PFCharacterStatTypes.generated.h"

struct FPFStatRowBinding;

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFCharacterStatValues
{
	GENERATED_BODY()

	static const TArray<FPFStatRowBinding>& GetRowBindings();

	FPFCharacterStatValues& operator+=(
		const FPFCharacterStatValues& Other);

	FPFCharacterStatValues operator+(
		const FPFCharacterStatValues& Other) const;

	bool TryApplyPercentModifiers(
		const FPFCharacterStatValues& PercentModifiers);

	UPROPERTY(
		EditAnywhere,
		Category = "Character Stat",
		meta = (DisplayName = "Attack"))
	float m_Attack = 0.0f;

	UPROPERTY(
		EditAnywhere,
		Category = "Character Stat",
		meta = (DisplayName = "Physical Defense"))
	float m_PhysicalDefense = 0.0f;

	UPROPERTY(
		EditAnywhere,
		Category = "Character Stat",
		meta = (DisplayName = "Magical Defense"))
	float m_MagicalDefense = 0.0f;

	UPROPERTY(
		EditAnywhere,
		Category = "Character Stat",
		meta = (DisplayName = "Turn Speed"))
	float m_TurnSpeed = 0.0f;

	UPROPERTY(
		EditAnywhere,
		Category = "Character Stat",
		meta = (DisplayName = "Max HP"))
	float m_MaxHP = 0.0f;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Character Stat",
		meta = (
			ClampMin = "0.0",
			DisplayName = "Max Ultimate Gauge"))
	float m_MaxUltimateGauge = 0.0f;
};

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFCharacterStatModifiers
{
	GENERATED_BODY()

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Character Stat Modifier",
		meta = (DisplayName = "Flat Modifiers"))
	FPFCharacterStatValues m_FlatModifiers;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Character Stat Modifier",
		meta = (DisplayName = "Percent Modifiers"))
	FPFCharacterStatValues m_PercentModifiers;
};

struct FPFStatRowBinding
{
	FName m_RowName;

	float FPFCharacterStatValues::* m_ValueMember = nullptr;
};
