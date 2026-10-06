#include "PFBattleAttributeSet.h"

#include "GameplayEffectExtension.h"

void UPFBattleAttributeSet::PostGameplayEffectExecute(
	const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& ModifiedAttribute =
		Data.EvaluatedData.Attribute;

	if (ModifiedAttribute == GetHPAttribute())
	{
		SetHP(FMath::Clamp(
			GetHP(),
			0.0f,
			GetMaxHP()));
	}

	if (ModifiedAttribute == GetMaxHPAttribute())
	{
		SetMaxHP(FMath::Max(
			GetMaxHP(),
			0.0f));

		SetHP(FMath::Clamp(
			GetHP(),
			0.0f,
			GetMaxHP()));
	}

	if (ModifiedAttribute == GetUltimateGaugeAttribute())
	{
		SetUltimateGauge(FMath::Clamp(
			GetUltimateGauge(),
			0.0f,
			GetMaxUltimateGauge()));
	}

	if (ModifiedAttribute == GetMaxUltimateGaugeAttribute())
	{
		SetMaxUltimateGauge(FMath::Max(
			GetMaxUltimateGauge(),
			0.0f));

		SetUltimateGauge(FMath::Clamp(
			GetUltimateGauge(),
			0.0f,
			GetMaxUltimateGauge()));
	}
}
