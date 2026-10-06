#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "PFBattleDamageExecutionCalculation.generated.h"

UCLASS()
class PROJECT_SIH_API UPFBattleDamageExecutionCalculation
	: public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UPFBattleDamageExecutionCalculation();

protected:
	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
