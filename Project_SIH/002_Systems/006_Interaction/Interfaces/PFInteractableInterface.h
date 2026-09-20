#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PFInteractableInterface.generated.h"

class APlayerController;

UINTERFACE(MinimalAPI)
class UPFInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 플레이어의 상호작용 요청을 처리하는 공용 계약 인터페이스입니다.
 */
class PROJECT_SIH_API IPFInteractableInterface
{
	GENERATED_BODY()

public:
	virtual bool Interact(APlayerController* Interactor) = 0;
};
