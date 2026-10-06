#include "PFPlayerController.h"

#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "Project_SIH/002_Systems/006_Interaction/PFInteractionComponent.h"

APFPlayerController::APFPlayerController()
{
	m_InteractionComponent = CreateDefaultSubobject<UPFInteractionComponent>(TEXT("InteractionComponent"));
}

void APFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 임시로 F키에 상호작용을 바인딩합니다.
	if (IsValid(InputComponent))
	{
		InputComponent->BindKey(EKeys::F, IE_Pressed, this, &APFPlayerController::HandleInteractInput);
	}
}

bool APFPlayerController::TryInteract()
{
	return IsValid(m_InteractionComponent) && m_InteractionComponent->TryInteract();
}

void APFPlayerController::HandleInteractInput()
{
	TryInteract();
}
