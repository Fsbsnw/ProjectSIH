#include "PFInteractionComponent.h"

#include "GameFramework/PlayerController.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"

UPFInteractionComponent::UPFInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPFInteractionComponent::TryRegisterInteractable(
	const TScriptInterface<IPFInteractableInterface>& Interactable)
{
	UObject* InteractableObject = Interactable.GetObject();
	if (!IsValid(InteractableObject) || Interactable.GetInterface() == nullptr)
	{
		return false;
	}

	UObject* CurrentInteractableObject = m_CurrentInteractable.GetObject();
	if (IsValid(CurrentInteractableObject) && CurrentInteractableObject != InteractableObject)
	{
		PF_LOG(
			TEXT("Overlapping interaction ranges are not supported. Current=%s, Rejected=%s"),
			*GetNameSafe(CurrentInteractableObject),
			*GetNameSafe(InteractableObject));
		return false;
	}

	m_CurrentInteractable = Interactable;
	return true;
}

void UPFInteractionComponent::UnregisterInteractable(
	const TScriptInterface<IPFInteractableInterface>& Interactable)
{
	if (m_CurrentInteractable.GetObject() == Interactable.GetObject())
	{
		m_CurrentInteractable = nullptr;
	}
}

bool UPFInteractionComponent::TryInteract()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!IsValid(PlayerController))
	{
		PF_LOG(TEXT("InteractionComponent owner is not a PlayerController"));
		return false;
	}

	UObject* InteractableObject = m_CurrentInteractable.GetObject();
	IPFInteractableInterface* Interactable = m_CurrentInteractable.GetInterface();
	if (!IsValid(InteractableObject) || Interactable == nullptr)
	{
		m_CurrentInteractable = nullptr;
		return false;
	}

	return Interactable->Interact(PlayerController);
}
