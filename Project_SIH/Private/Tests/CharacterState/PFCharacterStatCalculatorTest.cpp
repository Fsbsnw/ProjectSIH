#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Curves/SimpleCurve.h"
#include "Engine/CurveTable.h"
#include "Misc/AutomationTest.h"
#include "Project_SIH/000_Core/001_Contracts/004_Character/PFCharacterStatTypes.h"
#include "Project_SIH/002_Systems/004_CharacterState/Calculation/PFCharacterStatCalculator.h"

#include <limits>

namespace
{
	constexpr float StatTestTolerance = 0.01f;

	void AddLevelBonusCurve(
		UCurveTable& CurveTable,
		const FName RowName,
		const float LevelOneBonus,
		const float LevelTwoBonus,
		const bool bAddKeys = true)
	{
		FSimpleCurve& Curve = CurveTable.AddSimpleCurve(RowName);
		Curve.PreInfinityExtrap = RCCE_Constant;
		Curve.PostInfinityExtrap = RCCE_Constant;

		if (bAddKeys)
		{
			Curve.AddKey(1.0f, LevelOneBonus);
			Curve.AddKey(2.0f, LevelTwoBonus);
		}
	}

	UCurveTable* CreateLevelBonusTable(
		const FPFCharacterStatValues& LevelOneBonuses,
		const FPFCharacterStatValues& LevelTwoBonuses,
		const FName OmittedRowName = NAME_None,
		const FName EmptyRowName = NAME_None)
	{
		UCurveTable* LevelBonusTable = NewObject<UCurveTable>();

		for (const FPFStatRowBinding& Binding
			: FPFCharacterStatValues::GetRowBindings())
		{
			if (Binding.m_RowName == OmittedRowName)
			{
				continue;
			}

			AddLevelBonusCurve(
				*LevelBonusTable,
				Binding.m_RowName,
				LevelOneBonuses.*Binding.m_ValueMember,
				LevelTwoBonuses.*Binding.m_ValueMember,
				Binding.m_RowName != EmptyRowName);
		}

		return LevelBonusTable;
	}

	FPFCharacterStatValues CreateBaseStats()
	{
		FPFCharacterStatValues Stats;
		Stats.m_Attack = 100.0f;
		Stats.m_PhysicalDefense = 20.0f;
		Stats.m_MagicalDefense = 30.0f;
		Stats.m_TurnSpeed = 40.0f;
		Stats.m_MaxHP = 1000.0f;
		Stats.m_MaxUltimateGauge = 100.0f;
		return Stats;
	}

	FPFCharacterStatValues CreateLevelOneBonuses()
	{
		FPFCharacterStatValues Stats;
		Stats.m_Attack = 10.0f;
		Stats.m_PhysicalDefense = 1.0f;
		Stats.m_MagicalDefense = 3.0f;
		Stats.m_TurnSpeed = 5.0f;
		Stats.m_MaxHP = 100.0f;
		Stats.m_MaxUltimateGauge = 10.0f;
		return Stats;
	}

	FPFCharacterStatValues CreateLevelTwoBonuses()
	{
		FPFCharacterStatValues Stats;
		Stats.m_Attack = 20.0f;
		Stats.m_PhysicalDefense = 2.0f;
		Stats.m_MagicalDefense = 4.0f;
		Stats.m_TurnSpeed = 6.0f;
		Stats.m_MaxHP = 200.0f;
		Stats.m_MaxUltimateGauge = 20.0f;
		return Stats;
	}

	bool AreStatValuesNearlyEqual(
		const FPFCharacterStatValues& Left,
		const FPFCharacterStatValues& Right)
	{
		for (const FPFStatRowBinding& Binding
			: FPFCharacterStatValues::GetRowBindings())
		{
			if (!FMath::IsNearlyEqual(
				Left.*Binding.m_ValueMember,
				Right.*Binding.m_ValueMember,
				StatTestTolerance))
			{
				return false;
			}
		}

		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPFCharacterStatCalculatorTest,
	"Project_SIH.CharacterState.StatCalculator.BaseLevelAndEquipment",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::ProductFilter)

bool FPFCharacterStatCalculatorTest::RunTest(
	const FString& Parameters)
{
	const FPFCharacterStatValues BaseStats = CreateBaseStats();
	const FPFCharacterStatValues LevelOneBonuses =
		CreateLevelOneBonuses();
	const FPFCharacterStatValues LevelTwoBonuses =
		CreateLevelTwoBonuses();

	UCurveTable* LevelBonusTable =
		CreateLevelBonusTable(
			LevelOneBonuses,
			LevelTwoBonuses);

	const FPFCharacterStatModifiers NoEquipmentModifiers;

	FPFCharacterStatValues NoEquipmentStats;
	const bool bNoEquipmentCalculated =
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			LevelBonusTable,
			2,
			NoEquipmentModifiers,
			NoEquipmentStats);

	if (!TestTrue(
		TEXT("장비가 없을 때 스탯 계산이 성공해야 합니다."),
		bNoEquipmentCalculated))
	{
		return false;
	}

	TestTrue(
		TEXT("장비가 없을 때 공격력은 120이어야 합니다."),
		FMath::IsNearlyEqual(
			NoEquipmentStats.m_Attack,
			120.0f,
			StatTestTolerance));

	TestTrue(
		TEXT("장비가 없을 때 물리 방어력은 22여야 합니다."),
		FMath::IsNearlyEqual(
			NoEquipmentStats.m_PhysicalDefense,
			22.0f,
			StatTestTolerance));

	TestTrue(
		TEXT("장비가 없을 때 마법 방어력은 34여야 합니다."),
		FMath::IsNearlyEqual(
			NoEquipmentStats.m_MagicalDefense,
			34.0f,
			StatTestTolerance));

	TestTrue(
		TEXT("장비가 없을 때 턴 속도는 46이어야 합니다."),
		FMath::IsNearlyEqual(
			NoEquipmentStats.m_TurnSpeed,
			46.0f,
			StatTestTolerance));

	TestTrue(
		TEXT("장비가 없을 때 최대 체력은 1200이어야 합니다."),
		FMath::IsNearlyEqual(
			NoEquipmentStats.m_MaxHP,
			1200.0f,
			StatTestTolerance));

	TestTrue(
		TEXT("장비가 없을 때 최대 궁극기 게이지는 120이어야 합니다."),
		FMath::IsNearlyEqual(
			NoEquipmentStats.m_MaxUltimateGauge,
			120.0f,
			StatTestTolerance));

	FPFCharacterStatModifiers FirstEquipmentModifiers;
	FirstEquipmentModifiers.m_FlatModifiers.m_Attack = 10.0f;
	FirstEquipmentModifiers
		.m_FlatModifiers.m_PhysicalDefense = 3.0f;
	FirstEquipmentModifiers
		.m_FlatModifiers.m_MaxUltimateGauge = 10.0f;
	FirstEquipmentModifiers.m_PercentModifiers.m_Attack = 0.1f;
	FirstEquipmentModifiers.m_PercentModifiers.m_TurnSpeed = 0.2f;

	FPFCharacterStatModifiers SecondEquipmentModifiers;
	SecondEquipmentModifiers.m_FlatModifiers.m_Attack = 20.0f;
	SecondEquipmentModifiers.m_FlatModifiers.m_MaxHP = 100.0f;
	SecondEquipmentModifiers.m_PercentModifiers.m_Attack = 0.2f;
	SecondEquipmentModifiers.m_PercentModifiers.m_MaxHP = 0.1f;
	SecondEquipmentModifiers
		.m_PercentModifiers.m_MaxUltimateGauge = 0.1f;

	FPFCharacterStatModifiers EquipmentModifiers;
	EquipmentModifiers.m_FlatModifiers =
		FirstEquipmentModifiers.m_FlatModifiers
		+ SecondEquipmentModifiers.m_FlatModifiers;
	EquipmentModifiers.m_PercentModifiers =
		FirstEquipmentModifiers.m_PercentModifiers
		+ SecondEquipmentModifiers.m_PercentModifiers;

	FPFCharacterStatValues EquipmentStats;
	const bool bEquipmentCalculated =
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			LevelBonusTable,
			2,
			EquipmentModifiers,
			EquipmentStats);

	if (!TestTrue(
		TEXT("장비 보정이 있을 때 스탯 계산이 성공해야 합니다."),
		bEquipmentCalculated))
	{
		return false;
	}

	TestTrue(
		TEXT("장비 보정 후 공격력은 195여야 합니다."),
		FMath::IsNearlyEqual(
			EquipmentStats.m_Attack,
			195.0f,
			StatTestTolerance));

	TestTrue(
		TEXT("장비 보정 후 물리 방어력은 25여야 합니다."),
		FMath::IsNearlyEqual(
			EquipmentStats.m_PhysicalDefense,
			25.0f,
			StatTestTolerance));

	TestTrue(
		TEXT("장비 보정 후 마법 방어력은 34여야 합니다."),
		FMath::IsNearlyEqual(
			EquipmentStats.m_MagicalDefense,
			34.0f,
			StatTestTolerance));

	TestTrue(
		TEXT("장비 보정 후 턴 속도는 55.2여야 합니다."),
		FMath::IsNearlyEqual(
			EquipmentStats.m_TurnSpeed,
			55.2f,
			StatTestTolerance));

	TestTrue(
		TEXT("장비 보정 후 최대 체력은 1430이어야 합니다."),
		FMath::IsNearlyEqual(
			EquipmentStats.m_MaxHP,
			1430.0f,
			StatTestTolerance));

	TestTrue(
		TEXT("장비 보정 후 최대 궁극기 게이지는 143이어야 합니다."),
		FMath::IsNearlyEqual(
			EquipmentStats.m_MaxUltimateGauge,
			143.0f,
			StatTestTolerance));

	FPFCharacterStatValues FirstEquipmentStats;
	const bool bFirstEquipmentCalculated =
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			LevelBonusTable,
			2,
			FirstEquipmentModifiers,
			FirstEquipmentStats);

	FPFCharacterStatValues SecondEquipmentStats;
	const bool bSecondEquipmentCalculated =
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			LevelBonusTable,
			2,
			SecondEquipmentModifiers,
			SecondEquipmentStats);

	if (TestTrue(
			TEXT("장비 교체 전후 계산이 모두 성공해야 합니다."),
			bFirstEquipmentCalculated
				&& bSecondEquipmentCalculated))
	{
		TestFalse(
			TEXT("서로 다른 장비로 교체하면 계산 결과가 달라야 합니다."),
			AreStatValuesNearlyEqual(
				FirstEquipmentStats,
				SecondEquipmentStats));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPFCharacterStatDeterminismAndBoundaryTest,
	"Project_SIH.CharacterState.StatCalculator.DeterminismAndBoundaries",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::ProductFilter)

bool FPFCharacterStatDeterminismAndBoundaryTest::RunTest(
	const FString& Parameters)
{
	const FPFCharacterStatValues BaseStats = CreateBaseStats();
	const FPFCharacterStatValues LevelOneBonuses =
		CreateLevelOneBonuses();
	const FPFCharacterStatValues LevelTwoBonuses =
		CreateLevelTwoBonuses();

	UCurveTable* LevelBonusTable =
		CreateLevelBonusTable(
			LevelOneBonuses,
			LevelTwoBonuses);

	const FPFCharacterStatModifiers NoEquipmentModifiers;

	FPFCharacterStatValues FirstResult;
	const bool bFirstCalculationSucceeded =
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			LevelBonusTable,
			2,
			NoEquipmentModifiers,
			FirstResult);

	FPFCharacterStatValues SecondResult;
	const bool bSecondCalculationSucceeded =
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			LevelBonusTable,
			2,
			NoEquipmentModifiers,
			SecondResult);

	if (TestTrue(
			TEXT("동일 입력의 두 계산이 모두 성공해야 합니다."),
			bFirstCalculationSucceeded
				&& bSecondCalculationSucceeded))
	{
		TestTrue(
			TEXT("동일 입력은 동일한 결과를 반환해야 합니다."),
			AreStatValuesNearlyEqual(
				FirstResult,
				SecondResult));
	}

	FPFCharacterStatValues LevelOneResult;
	const bool bLevelOneCalculationSucceeded =
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			LevelBonusTable,
			1,
			NoEquipmentModifiers,
			LevelOneResult);

	FPFCharacterStatValues ExpectedLevelOneResult;
	ExpectedLevelOneResult.m_Attack = 110.0f;
	ExpectedLevelOneResult.m_PhysicalDefense = 21.0f;
	ExpectedLevelOneResult.m_MagicalDefense = 33.0f;
	ExpectedLevelOneResult.m_TurnSpeed = 45.0f;
	ExpectedLevelOneResult.m_MaxHP = 1100.0f;
	ExpectedLevelOneResult.m_MaxUltimateGauge = 110.0f;

	if (TestTrue(
			TEXT("최소 레벨 1 계산이 성공해야 합니다."),
			bLevelOneCalculationSucceeded))
	{
		TestTrue(
			TEXT("레벨 1에서는 레벨 1 보너스를 적용해야 합니다."),
			AreStatValuesNearlyEqual(
				LevelOneResult,
				ExpectedLevelOneResult));
	}

	FPFCharacterStatValues LevelFiftyResult;
	const bool bLevelFiftyCalculationSucceeded =
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			LevelBonusTable,
			50,
			NoEquipmentModifiers,
			LevelFiftyResult);

	FPFCharacterStatValues ExpectedLevelFiftyResult;
	ExpectedLevelFiftyResult.m_Attack = 120.0f;
	ExpectedLevelFiftyResult.m_PhysicalDefense = 22.0f;
	ExpectedLevelFiftyResult.m_MagicalDefense = 34.0f;
	ExpectedLevelFiftyResult.m_TurnSpeed = 46.0f;
	ExpectedLevelFiftyResult.m_MaxHP = 1200.0f;
	ExpectedLevelFiftyResult.m_MaxUltimateGauge = 120.0f;

	if (TestTrue(
			TEXT("최대 레벨 50 계산이 성공해야 합니다."),
			bLevelFiftyCalculationSucceeded))
	{
		TestTrue(
			TEXT("마지막 Key 이후에는 Constant 외삽값을 적용해야 합니다."),
			AreStatValuesNearlyEqual(
				LevelFiftyResult,
				ExpectedLevelFiftyResult));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPFCharacterStatInvalidInputTest,
	"Project_SIH.CharacterState.StatCalculator.InvalidInputs",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::ProductFilter)

bool FPFCharacterStatInvalidInputTest::RunTest(
	const FString& Parameters)
{
	const FPFCharacterStatValues BaseStats = CreateBaseStats();
	const FPFCharacterStatValues LevelOneBonuses =
		CreateLevelOneBonuses();
	const FPFCharacterStatValues LevelTwoBonuses =
		CreateLevelTwoBonuses();

	UCurveTable* LevelBonusTable =
		CreateLevelBonusTable(
			LevelOneBonuses,
			LevelTwoBonuses);

	const FPFCharacterStatModifiers NoEquipmentModifiers;

	FPFCharacterStatValues Result;

	TestFalse(
		TEXT("Level Bonus Table이 없으면 계산에 실패해야 합니다."),
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			nullptr,
			1,
			NoEquipmentModifiers,
			Result));

	TestFalse(
		TEXT("최소 레벨보다 낮은 레벨은 계산에 실패해야 합니다."),
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			LevelBonusTable,
			0,
			NoEquipmentModifiers,
			Result));

	TestFalse(
		TEXT("최대 레벨보다 높은 레벨은 계산에 실패해야 합니다."),
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			LevelBonusTable,
			51,
			NoEquipmentModifiers,
			Result));

	const TArray<FPFStatRowBinding>& RowBindings =
		FPFCharacterStatValues::GetRowBindings();

	UCurveTable* MissingRowTable =
		CreateLevelBonusTable(
			LevelOneBonuses,
			LevelTwoBonuses,
			RowBindings.Last().m_RowName);

	TestFalse(
		TEXT("필수 Stat Row가 누락되면 계산에 실패해야 합니다."),
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			MissingRowTable,
			1,
			NoEquipmentModifiers,
			Result));

	UCurveTable* EmptyCurveTable =
		CreateLevelBonusTable(
			LevelOneBonuses,
			LevelTwoBonuses,
			NAME_None,
			RowBindings[0].m_RowName);

	TestFalse(
		TEXT("필수 Stat Curve가 비어 있으면 계산에 실패해야 합니다."),
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			EmptyCurveTable,
			1,
			NoEquipmentModifiers,
			Result));

	FPFCharacterStatValues NonFiniteBaseStats = BaseStats;
	NonFiniteBaseStats.m_Attack =
		std::numeric_limits<float>::infinity();

	TestFalse(
		TEXT("유한하지 않은 기본 Stat 값은 계산에 실패해야 합니다."),
		FPFCharacterStatCalculator::TryCalculate(
			NonFiniteBaseStats,
			LevelBonusTable,
			1,
			NoEquipmentModifiers,
			Result));

	FPFCharacterStatModifiers NonFiniteEquipmentModifiers;
	NonFiniteEquipmentModifiers.m_PercentModifiers.m_Attack =
		std::numeric_limits<float>::infinity();

	TestFalse(
		TEXT("유한하지 않은 장비 보정값은 계산에 실패해야 합니다."),
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			LevelBonusTable,
			1,
			NonFiniteEquipmentModifiers,
			Result));

	FPFCharacterStatValues BossStats;

	TestTrue(
		TEXT("Boss BaseStats 계산은 원본 값을 반환해야 합니다."),
		FPFCharacterStatCalculator::TryCalculate(
			BaseStats,
			BossStats)
			&& AreStatValuesNearlyEqual(
				BaseStats,
				BossStats));

	FPFCharacterStatValues InvalidBossBaseStats = BaseStats;
	InvalidBossBaseStats.m_Attack = -1.0f;

	TestFalse(
		TEXT("음수 Boss BaseStats는 계산에 실패해야 합니다."),
		FPFCharacterStatCalculator::TryCalculate(
			InvalidBossBaseStats,
			BossStats));

	return true;
}

#endif
