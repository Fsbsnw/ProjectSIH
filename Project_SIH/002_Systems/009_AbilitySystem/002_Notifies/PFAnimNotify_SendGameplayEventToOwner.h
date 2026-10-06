#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "PFAnimNotify_SendGameplayEventToOwner.generated.h"

UCLASS(meta = (DisplayName = "SendGameplayEventToOwner"))
class PROJECT_SIH_API UPFAnimNotify_SendGameplayEventToOwner
	: public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(
		EditAnywhere,
		Category = "Gameplay Event",
		meta = (DisplayName = "Gameplay Event Tag", Categories = "Event"))
	FGameplayTag m_GameplayEventTag;
};
