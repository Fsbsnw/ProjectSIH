#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PFBattleLayoutProvider.generated.h"

class ACameraActor;
struct FPFBattleSpawnLayout;

UINTERFACE(MinimalAPI)
class UPFBattleLayoutProvider : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_SIH_API IPFBattleLayoutProvider
{
	GENERATED_BODY()

public:
	virtual bool TryGetBattleSpawnLayout(
		FPFBattleSpawnLayout& OutLayout) const = 0;

	virtual bool TryGetBattleCamera(
		ACameraActor*& OutCamera) const = 0;
};
