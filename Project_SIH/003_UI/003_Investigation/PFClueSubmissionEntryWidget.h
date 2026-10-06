#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PFClueSubmissionEntryWidget.generated.h"

class UCommonButtonBase;
class UCommonTextBlock;

/** 단서 제출 목록의 단서 ID, 표시 문구와 공용 버튼 클릭을 연결하는 항목 위젯입니다. */
UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFClueSubmissionEntryWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FClueSelectedEvent, const FGameplayTag&);

	/** 이 항목이 표시하고 제출할 단서 ID를 설정합니다. */
	void SetClueID(const FGameplayTag& ClueID);

	/** 항목의 공용 버튼이 클릭됐을 때 단서 ID를 전달합니다. */
	FClueSelectedEvent& OnClueClicked();

protected:
	/** 공용 버튼 클릭을 Entry의 단서 선택 Event에 연결합니다. */
	virtual void NativeOnInitialized() override;

private:
	/** 현재 단서 ID를 부모 단서 제출 Screen에 전달합니다. */
	void HandleSubmissionButtonClicked();

	/** 프로젝트 공용 스타일 버튼 WBP를 배치하는 선택 버튼입니다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_ClueSubmission;

	/** 단서 GameplayTag 문자열을 표시하는 텍스트입니다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Clue;

	FGameplayTag m_ClueID;
	FClueSelectedEvent m_OnClueSelected;
};
