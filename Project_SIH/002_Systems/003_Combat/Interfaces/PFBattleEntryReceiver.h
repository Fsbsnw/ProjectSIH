#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Project_SIH/000_Core/001_Contracts/000_Flow/PFGameFlowTypes.h"
#include "PFBattleEntryReceiver.generated.h"

UINTERFACE(MinimalAPI)
class UPFBattleEntryReceiver : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_SIH_API IPFBattleEntryReceiver
{
	GENERATED_BODY()

public:
	virtual EPFPhaseStartResult StartBattle(
		const FPFBattleEntryContext& Context) = 0;
};
