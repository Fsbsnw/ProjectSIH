#include "PFBattleDamageExecutionCalculation.h"

#include "GameplayEffectTypes.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/003_Combat/002_Attributes/PFBattleAttributeSet.h"

namespace
{
struct FPFDamageCaptureStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Attack);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalDefense);

	FPFDamageCaptureStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(
			UPFBattleAttributeSet,
			Attack,
			Source,
			true);

		DEFINE_ATTRIBUTE_CAPTUREDEF(
			UPFBattleAttributeSet,
			PhysicalDefense,
			Target,
			false);
	}
};

const FPFDamageCaptureStatics& GetDamageCaptureStatics()
{
	static const FPFDamageCaptureStatics Statics;
	return Statics;
}
}

UPFBattleDamageExecutionCalculation::
UPFBattleDamageExecutionCalculation()
{
	const FPFDamageCaptureStatics& Captures =
		GetDamageCaptureStatics();

	RelevantAttributesToCapture.Add(Captures.AttackDef);
	RelevantAttributesToCapture.Add(Captures.PhysicalDefenseDef);
}

void UPFBattleDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& EffectSpec =
		ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags =
		EffectSpec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags =
		EffectSpec.CapturedTargetTags.GetAggregatedTags();

	const FPFDamageCaptureStatics& Captures =
		GetDamageCaptureStatics();

	float Attack = 0.0f;
	float PhysicalDefense = 0.0f;

	if (!ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
			Captures.AttackDef,
			EvaluationParameters,
			Attack)
		|| !ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
			Captures.PhysicalDefenseDef,
			EvaluationParameters,
			PhysicalDefense))
	{
		PF_LOG(TEXT("Failed to capture battle damage attributes"));
		return;
	}

	const float Damage =
		FMath::Max(Attack - PhysicalDefense, 1.0f);

	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(
			UPFBattleAttributeSet::GetHPAttribute(),
			EGameplayModOp::Additive,
			-Damage));
}
