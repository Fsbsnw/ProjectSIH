#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Project_SIH/002_Systems/006_Interaction/Interfaces/PFInteractableInterface.h"
#include "PFInteractableActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;

/**
 * 범위 기반 상호작용 대상의 공통 충돌 영역만 제공하는 부모 Actor입니다.
 */
UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API APFInteractableActor : public AActor, public IPFInteractableInterface
{
	GENERATED_BODY()

public:
	APFInteractableActor();
	virtual bool Interact(APlayerController* Interactor) override;

protected:
	/** 상호작용 범위에 진입한 플레이어에게 자신을 상호작용 대상으로 등록합니다. */
	UFUNCTION()
	void HandleInteractionRangeBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/** 상호작용 범위를 벗어난 플레이어에게서 자신을 상호작용 대상으로 해제합니다. */
	UFUNCTION()
	void HandleInteractionRangeEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UStaticMeshComponent> m_StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<USphereComponent> m_InteractionRange;
};
