#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/002_Systems/006_Interaction/PFInteractableActor.h"
#include "PFClueInteractableActor.generated.h"

enum class EPFClueAcquireResult : uint8;
struct FPFDialogueActionPresentation;
class UPFDialogueSubsystem;

UCLASS(Blueprintable)
class PROJECT_SIH_API APFClueInteractableActor : public APFInteractableActor
{
	GENERATED_BODY()

public:
	virtual bool Interact(APlayerController* Interactor) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool TryStartDialogue(APlayerController* Interactor);
	EPFClueAcquireResult TryAcquireClue() const;
	void BindDialogueEvents(UPFDialogueSubsystem* DialogueSubsystem);
	void UnbindDialogueEvents();
	void ExecuteAcquireClueAction(FName ActionNodeID);

	UFUNCTION()
	void HandleDialogueActionRequested(const FPFDialogueActionPresentation& Presentation);

	UFUNCTION()
	void HandleDialogueFinished();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clue", meta = (AllowPrivateAccess = "true", Categories = "ID.Clue"))
	FGameplayTag m_ClueID;

	/** 설정되어 있으면 즉시 단서를 획득하지 않고 해당 대화를 시작합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true", DisplayName = "Dialogue ID", Categories = "ID.Dialogue"))
	FGameplayTag m_DialogueID;

	/** 이 액터가 시작한 대화의 Action과 종료 Event를 구독하는 동안만 유지합니다. */
	UPROPERTY(Transient)
	TObjectPtr<UPFDialogueSubsystem> m_DialogueSubsystem;
};
