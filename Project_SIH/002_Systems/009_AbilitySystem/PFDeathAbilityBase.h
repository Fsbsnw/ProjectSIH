#pragma once

#include "CoreMinimal.h"
#include "PFAbilityBase.h"
#include "PFDeathAbilityBase.generated.h"

class UAnimMontage;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFDeathAbilityBase
	: public UPFAbilityBase
{
	GENERATED_BODY()

public:
	UPFDeathAbilityBase();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	UFUNCTION()
	void HandleDeathMontageCompleted();

	UFUNCTION()
	void HandleDeathMontageInterrupted();

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Battle|Death",
		meta = (
			AllowPrivateAccess = "true",
			DisplayName = "Death Montage"))
	TObjectPtr<UAnimMontage> m_DeathMontage;
};
