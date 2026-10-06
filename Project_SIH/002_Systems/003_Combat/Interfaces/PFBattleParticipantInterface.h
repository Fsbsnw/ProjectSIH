#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleTypes.h"
#include "PFBattleParticipantInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UPFBattleParticipantInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_SIH_API IPFBattleParticipantInterface
{
	GENERATED_BODY()

public:
	virtual EPFBattleSide GetBattleSide() const = 0;

	virtual bool IsAlive() const = 0;
};
