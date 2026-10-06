#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PFDeathParticipantInterface.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(
	FPFOnDeathStarted,
	AActor*);

DECLARE_MULTICAST_DELEGATE_OneParam(
	FPFOnDeathFinished,
	AActor*);

UINTERFACE(
	MinimalAPI,
	meta = (CannotImplementInterfaceInBlueprint))
class UPFDeathParticipantInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_SIH_API IPFDeathParticipantInterface
{
	GENERATED_BODY()

public:
	virtual bool HasPendingDeath() const = 0;

	virtual bool StartPendingDeath() = 0;

	virtual void NotifyDeathStarted() = 0;

	virtual void NotifyDeathFinished() = 0;

	virtual FPFOnDeathStarted&
		GetDeathStartedDelegate() = 0;

	virtual FPFOnDeathFinished&
		GetDeathFinishedDelegate() = 0;
};
