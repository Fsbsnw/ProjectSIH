#pragma once

#include "CoreMinimal.h"

class UCurveTable;
struct FPFCharacterStatModifiers;
struct FPFCharacterStatValues;

class PROJECT_SIH_API FPFCharacterStatCalculator
{
public:
	// Case별 Boss BaseStats Override를 최종 Stat으로 계산한다.
	// Level Curve와 장비 보정은 적용하지 않는다.
	static bool TryCalculate(
		const FPFCharacterStatValues& BaseStats,
		FPFCharacterStatValues& OutStats);

	static bool TryCalculate(
		const FPFCharacterStatValues& BaseStats,
		const UCurveTable* LevelBonusTable,
		int32 CharacterLevel,
		const FPFCharacterStatModifiers& EquipmentModifiers,
		FPFCharacterStatValues& OutStats);

private:
	static bool TryEvaluateLevelBonusStats(
		const UCurveTable* LevelBonusTable,
		int32 CharacterLevel,
		FPFCharacterStatValues& OutLevelBonus);

	static bool TryEvaluateLevelBonus(
		const UCurveTable* LevelBonusTable,
		FName RowName,
		int32 CharacterLevel,
		float& OutLevelBonus);
};
