#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/002_Systems/006_Interaction/PFInteractableActor.h"
#include "PFClueInteractableActor.generated.h"

UCLASS(Blueprintable)
class PROJECT_SIH_API APFClueInteractableActor : public APFInteractableActor
{
	GENERATED_BODY()

public:
	virtual bool Interact(APlayerController* Interactor) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clue", meta = (AllowPrivateAccess = "true", Categories = "ID.Clue"))
	FGameplayTag m_ClueID;
};
