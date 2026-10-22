#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "Project_SIH/000_Core/001_Contracts/006_Dialogue/PFDialogueTypes.h"
#include "PFDialogueChoiceEntryWidget.generated.h"

class UCommonButtonBase;
class UCommonTextBlock;

/** 선택지 문구와 공용 스타일 버튼을 연결하는 대화 선택지 항목입니다. */
UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFDialogueChoiceEntryWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FChoiceSelectedEvent, FName);

	/** 이 항목이 표시하고 전달할 선택지 데이터를 설정합니다. */
	void SetChoice(const FPFDialogueChoice& Choice);

	/** 항목 버튼이 클릭됐을 때 Choice ID를 전달합니다. */
	FChoiceSelectedEvent& OnChoiceClicked();

protected:
	virtual void NativeOnInitialized() override;

private:
	void HandleChoiceButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Choice;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Choice;

	FName m_ChoiceID;
	FChoiceSelectedEvent m_OnChoiceSelected;
};
