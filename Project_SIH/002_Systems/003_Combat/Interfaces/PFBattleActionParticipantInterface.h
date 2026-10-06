#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleTypes.h"
#include "PFBattleActionParticipantInterface.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(
	FPFOnBattleActionCompleted,
	AActor*);

UINTERFACE(
	MinimalAPI,
	meta = (CannotImplementInterfaceInBlueprint))
class UPFBattleActionParticipantInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_SIH_API IPFBattleActionParticipantInterface
{
	GENERATED_BODY()

public:
	virtual bool RequestBattleAction(
		const FPFBattleActionRequest& Request) = 0;

	virtual void NotifyBattleActionCompleted() = 0;

	virtual FPFOnBattleActionCompleted&
		GetBattleActionCompletedDelegate() = 0;
};
