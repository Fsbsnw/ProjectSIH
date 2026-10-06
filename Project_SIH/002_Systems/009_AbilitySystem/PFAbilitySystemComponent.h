#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleTypes.h"
#include "PFAbilitySystemComponent.generated.h"

UCLASS()
class PROJECT_SIH_API UPFAbilitySystemComponent
	: public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	bool ActivateBattleAction(
		const FPFBattleActionRequest& Request);

	bool HasBattleAction(
		FGameplayTag ActionTag) const;

	bool GetBattleActionTargetRule(
		FGameplayTag ActionTag,
		FPFBattleTargetRule& OutRule) const;

	bool ActivateDeathAbility();

	bool HasDeathAbility() const;

	bool AddLooseGameplayTagIfNone(
		FGameplayTag GameplayTag);

	bool RemoveLooseGameplayTagIfExists(
		FGameplayTag GameplayTag);

protected:
	virtual void OnGiveAbility(
		FGameplayAbilitySpec& AbilitySpec) override;

	virtual void OnRemoveAbility(
		FGameplayAbilitySpec& AbilitySpec) override;

private:
	TMap<FGameplayTag, FGameplayAbilitySpecHandle>
		m_BattleActionAbilityHandles;

	FGameplayAbilitySpecHandle m_DeathAbilityHandle;
};
