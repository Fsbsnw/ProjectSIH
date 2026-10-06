#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "PFBattleActionAbilityBase.h"
#include "PFBasicAttackAbility.generated.h"

class UAbilitySystemComponent;
class UAnimMontage;
class UPFEffectBase;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFBasicAttackAbility
	: public UPFBattleActionAbilityBase
{
	GENERATED_BODY()

public:
	UPFBasicAttackAbility();

	virtual FGameplayTag GetActionTag() const override;

	virtual bool CanActivateBattleAction(
		const FPFBattleActionRequest& Request) const override;

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
	void HandleHitEvent(FGameplayEventData Payload);

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageInterrupted();

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Battle|Basic Attack",
		meta = (DisplayName = "Attack Montage"))
	TObjectPtr<UAnimMontage> m_AttackMontage;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Battle|Basic Attack",
		meta = (DisplayName = "Damage Effect"))
	TSubclassOf<UPFEffectBase> m_DamageEffect;

	TWeakObjectPtr<UAbilitySystemComponent> m_TargetASC;
};
