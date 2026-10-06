#pragma once

#include "CoreMinimal.h"
#include "PFAbilityBase.h"
#include "PFBattleActionAbilityInterface.h"
#include "PFBattleActionAbilityBase.generated.h"

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFBattleActionAbilityBase
	: public UPFAbilityBase
	, public IPFBattleActionAbilityInterface
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetActionTag() const override;

	virtual FPFBattleTargetRule GetTargetRule() const override;

	virtual bool CanActivateBattleAction(
		const FPFBattleActionRequest& Request) const override;

protected:
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Battle|Target",
		meta = (DisplayName = "Target Rule"))
	FPFBattleTargetRule m_TargetRule;
};
