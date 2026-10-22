#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFMeetingTypes.h"
#include "Project_SIH/003_UI/000_Foundation/PFActivatableWidget.h"
#include "PFClueSubmissionWidget.generated.h"

class UVerticalBox;
class UPFClueSubmissionEntryWidget;

/**
 * Meeting 단서 제출 테스트용 Screen 기반입니다.
 * 임시 주장 목록에서 하나를 선택한 뒤 실제 획득 단서만 표시하여 제출합니다.
 */
UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFClueSubmissionWidget : public UPFActivatableWidget
{
	GENERATED_BODY()

public:
	/** 현재 Meeting에서 제출 가능한 획득 단서 목록을 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "UI|Meeting")
	bool TryGetMeetingClueIDs(FGameplayTagContainer& OutClueIDs) const;

	/** 현재 Meeting에 주장을 선택합니다. 성공하면 획득 단서 선택 단계가 됩니다. */
	UFUNCTION(BlueprintCallable, Category = "UI|Meeting")
	EPFMeetingClaimSelectResult TrySelectClaim(FGameplayTag ClaimID);

	/** 선택한 주장에 현재 획득 단서를 제출합니다. */
	UFUNCTION(BlueprintCallable, Category = "UI|Meeting")
	EPFMeetingSubmitResult TrySubmitClue(FGameplayTag ClueID);

protected:
	/** Screen이 열릴 때 임시 주장 목록으로 VerticalBox를 구성합니다. */
	virtual void NativeOnActivated() override;

private:
	/** 테스트용 주장 두 개로 선택 목록을 구성합니다. */
	void RefreshClaimList();

	/** 현재 제출 가능한 단서를 조회하고 지정된 버튼 WBP를 생성합니다. */
	void RefreshClueList();

	/** 주장을 선택하고 성공하면 같은 목록을 획득 단서로 교체합니다. */
	void HandleClaimClicked(const FGameplayTag& ClaimID);

	/** 선택한 주장에 획득 단서를 제출하고 판정 결과를 대화로 전달합니다. */
	void HandleClueClicked(const FGameplayTag& ClueID);

	/** 생성된 단서 버튼을 세로로 배치할 VerticalBox입니다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_ClueList;

	/** VerticalBox의 주장과 단서를 표시할 공용 Entry WBP 클래스입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (DisplayName = "Submission Entry Class"))
	TSubclassOf<UPFClueSubmissionEntryWidget> m_ClueEntryClass;
};
