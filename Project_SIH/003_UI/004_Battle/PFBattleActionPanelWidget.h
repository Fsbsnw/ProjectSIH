#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PFBattleActionPanelWidget.generated.h"

class UCommonButtonBase;
class UCommonTextBlock;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFBattleActionPanelWidget
	: public UCommonUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FBasicAttackClickedEvent);
	DECLARE_MULTICAST_DELEGATE(FCancelClickedEvent);

	void SetControls(
		bool bCanSelectAction,
		bool bSelectingTarget);

	void SetHintText(const FText& HintText);

	FBasicAttackClickedEvent& OnBasicAttackClicked();
	FCancelClickedEvent& OnCancelClicked();

protected:
	virtual void NativeOnInitialized() override;

private:
	void HandleBasicAttackClicked();
	void HandleCancelClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Attack;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Cancel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Attack;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Hint;

	FBasicAttackClickedEvent m_OnBasicAttackClicked;
	FCancelClickedEvent m_OnCancelClicked;
};
