#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Project_SIH/002_Systems/006_Interaction/Interfaces/PFInteractableInterface.h"
#include "PFInteractionComponent.generated.h"

/**
 * 범위 안에 들어온 단일 상호작용 대상을 보관하고 상호작용 요청을 전달합니다.
 */
UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class PROJECT_SIH_API UPFInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPFInteractionComponent();

	/** 상호작용 가능한 대상을 현재 상호작용 대상으로 등록합니다. */
	bool TryRegisterInteractable(const TScriptInterface<IPFInteractableInterface>& Interactable);

	/** 등록된 상호작용 대상을 해제합니다. */
	void UnregisterInteractable(const TScriptInterface<IPFInteractableInterface>& Interactable);

	bool TryInteract();

private:
	/** 현재 상호작용 가능한 대상의 인터페이스입니다. */
	UPROPERTY(Transient)
	TScriptInterface<IPFInteractableInterface> m_CurrentInteractable;
};
