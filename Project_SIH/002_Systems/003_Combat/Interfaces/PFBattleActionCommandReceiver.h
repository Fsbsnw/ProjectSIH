#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleTypes.h"
#include "PFBattleActionCommandReceiver.generated.h"

UINTERFACE(
	MinimalAPI,
	meta = (CannotImplementInterfaceInBlueprint))
class UPFBattleActionCommandReceiver : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_SIH_API IPFBattleActionCommandReceiver
{
	GENERATED_BODY()

public:
	virtual bool GetValidTargetsAndDefaultTarget(
		APFBattleCharacterBase* Requester,
		FGameplayTag ActionTag,
		FPFBattleTargetSelection& OutSelection) const = 0;

	virtual bool SelectBattleAction(
		APFBattleCharacterBase* Requester,
		FGameplayTag ActionTag) = 0;

	virtual bool SelectBattleTarget(
		APFBattleCharacterBase* Requester,
		APFBattleCharacterBase* Target) = 0;

	virtual bool ConfirmBattleAction(
		APFBattleCharacterBase* Requester) = 0;

	virtual bool CancelBattleActionSelection(
		APFBattleCharacterBase* Requester) = 0;
};
