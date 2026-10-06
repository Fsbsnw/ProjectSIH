#include "PFGameUIRootBase.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Input/CommonUIInputTypes.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/003_UI/002_Subsystem/PFUIManagerSubsystem.h"

void UPFGameUIRootBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	APlayerController* PlayerController = GetOwningPlayer();
	ULocalPlayer* LocalPlayer = IsValid(PlayerController)
		? PlayerController->GetLocalPlayer()
		: nullptr;
	UPFUIManagerSubsystem* UIManager = IsValid(LocalPlayer)
		? LocalPlayer->GetSubsystem<UPFUIManagerSubsystem>()
		: nullptr;
	if (!IsValid(UIManager))
	{
		PF_LOG(TEXT("UIManagerSubsystem is not available for UI input registration."));
		return;
	}

	TArray<FPFUIInputBinding> InputBindings;
	UIManager->GetResolvedInputBindings(InputBindings);
	for (const FPFUIInputBinding& InputBinding : InputBindings)
	{
		RegisterWidgetInput(InputBinding.m_InputAction, InputBinding.m_WidgetID);
	}
}

void UPFGameUIRootBase::RegisterWidgetInput(
	const FDataTableRowHandle& InputAction,
	FGameplayTag WidgetID)
{
	if (InputAction.IsNull() || !WidgetID.IsValid())
	{
		return;
	}

	FBindUIActionArgs BindArgs(
		InputAction,
		false,
		FSimpleDelegate::CreateUObject(
			this,
			&UPFGameUIRootBase::HandleWidgetInput,
			WidgetID));
	BindArgs.InputMode = ECommonInputMode::All;
	BindArgs.bConsumeInput = true;
	RegisterUIActionBinding(BindArgs);
}

void UPFGameUIRootBase::HandleWidgetInput(FGameplayTag WidgetID)
{
	APlayerController* PlayerController = GetOwningPlayer();
	ULocalPlayer* LocalPlayer = IsValid(PlayerController)
		? PlayerController->GetLocalPlayer()
		: nullptr;
	UPFUIManagerSubsystem* UIManager = IsValid(LocalPlayer)
		? LocalPlayer->GetSubsystem<UPFUIManagerSubsystem>()
		: nullptr;
	if (!IsValid(UIManager))
	{
		PF_LOG(TEXT("UIManagerSubsystem is not available. WidgetID=%s"), *WidgetID.ToString());
		return;
	}

	if (!IsValid(UIManager->ShowWidget(WidgetID)))
	{
		PF_LOG(TEXT("Configured UI Widget could not be shown. WidgetID=%s"), *WidgetID.ToString());
	}
}
