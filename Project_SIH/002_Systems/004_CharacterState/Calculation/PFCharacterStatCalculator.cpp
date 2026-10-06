#include "PFCharacterStatCalculator.h"

#include "Curves/RealCurve.h"
#include "Engine/CurveTable.h"
#include "Project_SIH/000_Core/001_Contracts/004_Character/PFCharacterStatTypes.h"

namespace
{
	constexpr int32 MinCharacterLevel = 1;
	constexpr int32 MaxCharacterLevel = 50;
}

bool FPFCharacterStatCalculator::TryCalculate(
	const FPFCharacterStatValues& BaseStats,
	FPFCharacterStatValues& OutStats)
{
	OutStats = FPFCharacterStatValues();

	for (const FPFStatRowBinding& Binding
		: FPFCharacterStatValues::GetRowBindings())
	{
		if (BaseStats.*Binding.m_ValueMember < 0.0f)
		{
			return false;
		}
	}

	OutStats = BaseStats;
	return true;
}

bool FPFCharacterStatCalculator::TryCalculate(
	const FPFCharacterStatValues& BaseStats,
	const UCurveTable* LevelBonusTable,
	const int32 CharacterLevel,
	const FPFCharacterStatModifiers& EquipmentModifiers,
	FPFCharacterStatValues& OutStats)
{
	OutStats = FPFCharacterStatValues();

	if (!IsValid(LevelBonusTable)
		|| CharacterLevel < MinCharacterLevel
		|| CharacterLevel > MaxCharacterLevel)
	{
		return false;
	}

	FPFCharacterStatValues LevelBonus;

	if (!TryEvaluateLevelBonusStats(
		LevelBonusTable,
		CharacterLevel,
		LevelBonus))
	{
		return false;
	}

	FPFCharacterStatValues Result =
		BaseStats + LevelBonus;

	Result += EquipmentModifiers.m_FlatModifiers;

	if (!Result.TryApplyPercentModifiers(
		EquipmentModifiers.m_PercentModifiers))
	{
		return false;
	}

	OutStats = Result;
	return true;
}

bool FPFCharacterStatCalculator::TryEvaluateLevelBonusStats(
	const UCurveTable* LevelBonusTable,
	const int32 CharacterLevel,
	FPFCharacterStatValues& OutLevelBonus)
{
	OutLevelBonus = FPFCharacterStatValues();

	for (const FPFStatRowBinding& Binding
		: FPFCharacterStatValues::GetRowBindings())
	{
		float& LevelBonusValue =
			OutLevelBonus.*Binding.m_ValueMember;

		if (!TryEvaluateLevelBonus(
			LevelBonusTable,
			Binding.m_RowName,
			CharacterLevel,
			LevelBonusValue))
		{
			OutLevelBonus = FPFCharacterStatValues();
			return false;
		}
	}

	return true;
}

bool FPFCharacterStatCalculator::TryEvaluateLevelBonus(
	const UCurveTable* LevelBonusTable,
	const FName RowName,
	const int32 CharacterLevel,
	float& OutLevelBonus)
{
	const FRealCurve* LevelBonusCurve =
		LevelBonusTable->FindCurve(
			RowName,
			TEXT("FPFCharacterStatCalculator"),
			false);

	if (!LevelBonusCurve || LevelBonusCurve->GetNumKeys() == 0)
	{
		return false;
	}

	const float LevelBonus =
		LevelBonusCurve->Eval(
			static_cast<float>(CharacterLevel));

	if (!FMath::IsFinite(LevelBonus))
	{
		return false;
	}

	OutLevelBonus = LevelBonus;
	return true;
}
