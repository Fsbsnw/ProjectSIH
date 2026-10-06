#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PFPlayerController.generated.h"

class UPFInteractionComponent;

UCLASS()
class PROJECT_SIH_API APFPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APFPlayerController();

	bool TryInteract();

protected:
	virtual void SetupInputComponent() override;

private:
	void HandleInteractInput();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPFInteractionComponent> m_InteractionComponent;
};
