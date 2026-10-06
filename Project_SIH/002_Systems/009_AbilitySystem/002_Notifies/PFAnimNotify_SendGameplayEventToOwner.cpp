#include "PFAnimNotify_SendGameplayEventToOwner.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"

void UPFAnimNotify_SendGameplayEventToOwner::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)
		|| !IsValid(MeshComp->GetWorld())
		|| !MeshComp->GetWorld()->IsGameWorld())
	{
		return;
	}

	if (!m_GameplayEventTag.IsValid())
	{
		PF_LOG(TEXT("Gameplay event notify has no event tag"));
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner))
	{
		PF_LOG(TEXT("Gameplay event notify has no owning actor"));
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = m_GameplayEventTag;
	Payload.Instigator = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		m_GameplayEventTag,
		Payload);
}
