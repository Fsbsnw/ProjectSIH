#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleTypes.h"
#include "PFBattleActionAbilityInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UPFBattleActionAbilityInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_SIH_API IPFBattleActionAbilityInterface
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetActionTag() const = 0;

	virtual FPFBattleTargetRule GetTargetRule() const = 0;

	virtual bool CanActivateBattleAction(
		const FPFBattleActionRequest& Request) const = 0;
};
