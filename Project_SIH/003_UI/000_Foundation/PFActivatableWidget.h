#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CommonInputModeTypes.h"
#include "Engine/EngineBaseTypes.h"
#include "Input/UIActionBindingHandle.h"
#include "PFActivatableWidget.generated.h"

/**
 * SIH의 CommonUI 화면이 공통으로 상속하는 기반 위젯입니다.
 * 화면별 입력 모드는 클래스 기본값으로 설정하며 CommonUI의 활성화와 Back 동작을 사용합니다.
 */
UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/** CommonUI 화면에 필요한 기본 Back 처리와 포커스 복원 정책을 설정합니다. */
	UPFActivatableWidget(const FObjectInitializer& ObjectInitializer);

	/** 현재 위젯의 입력 모드 설정을 CommonUI Action Router에 제공합니다. */
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

private:
	/** 활성화 중 사용할 입력 경로입니다. Menu는 CommonUI 입력만 유지하고 이동·시점을 포함한 게임 입력을 차단합니다. */
	UPROPERTY(
		EditDefaultsOnly,
		Category = "Input",
		meta = (DisplayName = "Input Mode"))
	ECommonInputMode m_InputMode = ECommonInputMode::Menu;

	/** All 또는 Game 입력에서 사용할 마우스 캡처 정책입니다. Menu의 전체 게임 입력 차단에는 NoCapture를 사용합니다. */
	UPROPERTY(
		EditDefaultsOnly,
		Category = "Input",
		meta = (DisplayName = "Mouse Capture Mode"))
	EMouseCaptureMode m_MouseCaptureMode = EMouseCaptureMode::NoCapture;

	/** Viewport가 마우스를 캡처하는 동안 커서를 숨길지 결정합니다. */
	UPROPERTY(
		EditDefaultsOnly,
		Category = "Input",
		meta = (DisplayName = "Hide Cursor During Viewport Capture"))
	bool m_bHideCursorDuringViewportCapture = true;

	/** All 또는 Game 모드에서도 PlayerController의 이동 입력을 무시합니다. Menu 모드에서는 항상 적용됩니다. */
	UPROPERTY(
		EditDefaultsOnly,
		Category = "Input",
		meta = (DisplayName = "Ignore Input"))
	bool m_bIgnoreInput = false;
};
