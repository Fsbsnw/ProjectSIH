#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PFTurnParticipantInterface.generated.h"

UINTERFACE(
	MinimalAPI,
	meta = (CannotImplementInterfaceInBlueprint))
class UPFTurnParticipantInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_SIH_API IPFTurnParticipantInterface
{
	GENERATED_BODY()

public:
	virtual float GetTurnSpeed() const = 0;
};
