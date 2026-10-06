#include "PFActivatableWidget.h"

#include "Input/UIActionBindingHandle.h"

UPFActivatableWidget::UPFActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsBackHandler = true;
	bAutoRestoreFocus = true;
}

TOptional<FUIInputConfig> UPFActivatableWidget::GetDesiredInputConfig() const
{
	FUIInputConfig InputConfig(m_InputMode, m_MouseCaptureMode, m_bHideCursorDuringViewportCapture);
	const bool bBlockPlayerInput = m_InputMode == ECommonInputMode::Menu;

	InputConfig.bIgnoreMoveInput = bBlockPlayerInput || m_bIgnoreInput;
	InputConfig.bIgnoreLookInput = bBlockPlayerInput || m_bIgnoreInput;

	UE_LOG(LogTemp, Warning, TEXT("GetDesiredInputConfig Widget=%s Activated=%d"), *GetNameSafe(this), IsActivated());

	return InputConfig;
}
