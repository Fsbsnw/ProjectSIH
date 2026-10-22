#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/000_Core/001_Contracts/006_Dialogue/PFDialogueTypes.h"
#include "Project_SIH/003_UI/000_Foundation/PFActivatableWidget.h"
#include "PFDialogueWidget.generated.h"

class UCommonButtonBase;
class UCommonTextBlock;
class UImage;
class UVerticalBox;
class UPFDialogueChoiceEntryWidget;
class UPFDialogueSubsystem;

/**
 * 현재 대사와 선택지를 표시하는 테스트용 CommonUI 기반입니다.
 * Blueprint는 BindWidget 항목과 선택지 Entry 클래스를 구성하고 디자인만 담당합니다.
 */
UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFDialogueWidget : public UPFActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
	void BindDialogueEvents();
	void UnbindDialogueEvents();
	void RefreshFromCurrentState();
	void ClearChoiceEntries();
	void ResetPresentation();

	UFUNCTION()
	void HandleLinePresented(const FPFDialogueLinePresentation& Presentation);

	UFUNCTION()
	void HandleChoicesPresented(const FPFDialogueChoicePresentation& Presentation);

	UFUNCTION()
	void HandleActionRequested(const FPFDialogueActionPresentation& Presentation);

	UFUNCTION()
	void HandleDialogueCompleted();

	UFUNCTION()
	void HandleDialogueFailed();

	UFUNCTION()
	void HandleDialogueCancelled();

	void HandleContinueClicked();
	void HandleChoiceClicked(FName ChoiceID);
	void HandleActionResultClicked(FName ResultID);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Portrait;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Speaker;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Dialogue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Continue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_Choices;

	UPROPERTY(EditDefaultsOnly, Category = "UI",
		meta = (DisplayName = "Choice Entry Class"))
	TSubclassOf<UPFDialogueChoiceEntryWidget> m_ChoiceEntryClass;

	/** 테스트 중에만 Action 결과 분기를 선택지 버튼으로 노출합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Testing",
		meta = (DisplayName = "Enable Action Test Controls"))
	bool m_bEnableActionTestControls = false;

	TWeakObjectPtr<UPFDialogueSubsystem> m_DialogueSubsystem;
	FName m_CurrentActionNodeID;
};
