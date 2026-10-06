#include "PFCharacterStatTypes.h"

const TArray<FPFStatRowBinding>&
FPFCharacterStatValues::GetRowBindings()
{
	static const TArray<FPFStatRowBinding> RowBindings = {
		{
			FName(TEXT("Attack")),
			&FPFCharacterStatValues::m_Attack
		},
		{
			FName(TEXT("PhysicalDefense")),
			&FPFCharacterStatValues::m_PhysicalDefense
		},
		{
			FName(TEXT("MagicalDefense")),
			&FPFCharacterStatValues::m_MagicalDefense
		},
		{
			FName(TEXT("TurnSpeed")),
			&FPFCharacterStatValues::m_TurnSpeed
		},
		{
			FName(TEXT("MaxHP")),
			&FPFCharacterStatValues::m_MaxHP
		},
		{
			FName(TEXT("MaxUltimateGauge")),
			&FPFCharacterStatValues::m_MaxUltimateGauge
		}
	};

	return RowBindings;
}

FPFCharacterStatValues&
FPFCharacterStatValues::operator+=(
	const FPFCharacterStatValues& Other)
{
	m_Attack += Other.m_Attack;
	m_PhysicalDefense += Other.m_PhysicalDefense;
	m_MagicalDefense += Other.m_MagicalDefense;
	m_TurnSpeed += Other.m_TurnSpeed;
	m_MaxHP += Other.m_MaxHP;
	m_MaxUltimateGauge += Other.m_MaxUltimateGauge;

	return *this;
}

FPFCharacterStatValues
FPFCharacterStatValues::operator+(
	const FPFCharacterStatValues& Other) const
{
	FPFCharacterStatValues Result = *this;
	Result += Other;
	return Result;
}

bool FPFCharacterStatValues::TryApplyPercentModifiers(
	const FPFCharacterStatValues& PercentModifiers)
{
	FPFCharacterStatValues Result = *this;

	for (const FPFStatRowBinding& Binding : GetRowBindings())
	{
		const auto ValueMember = Binding.m_ValueMember;

		const float CurrentValue =
			Result.*ValueMember;

		const float PercentModifier =
			PercentModifiers.*ValueMember;

		const float CalculatedValue =
			CurrentValue * (1.0f + PercentModifier);

		if (!FMath::IsFinite(PercentModifier)
			|| !FMath::IsFinite(CalculatedValue))
		{
			return false;
		}

		Result.*ValueMember = CalculatedValue;
	}

	*this = Result;
	return true;
}
