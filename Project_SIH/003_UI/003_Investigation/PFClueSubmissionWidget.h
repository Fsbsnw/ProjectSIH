#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFMeetingTypes.h"
#include "Project_SIH/003_UI/000_Foundation/PFActivatableWidget.h"
#include "PFClueSubmissionWidget.generated.h"

class UVerticalBox;
class UPFClueSubmissionEntryWidget;
class UPFDialogueGraphExecutor;

/**
 * Meeting 단서 제출 테스트용 Screen 기반입니다.
 * Blueprint가 지정한 단서 Entry 클래스로 획득 단서 목록을 구성하고 Claim·Clue를 연속 제출합니다.
 */
UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFClueSubmissionWidget : public UPFActivatableWidget
{
	GENERATED_BODY()

public:
	/** Supplies the Dialogue execution that is currently waiting for Evidence. */
	UFUNCTION(BlueprintCallable, Category = "UI|Meeting")
	void SetDialogueExecutor(UPFDialogueGraphExecutor* DialogueExecutor);

	/** 현재 Meeting에서 제출 가능한 획득 단서 목록을 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "UI|Meeting")
	bool TryGetMeetingClueIDs(FGameplayTagContainer& OutClueIDs) const;

	/** 단서 Definition의 Claim을 먼저 선택하고 같은 단서를 이어서 제출합니다. */
	UFUNCTION(
		BlueprintCallable,
		Category = "UI|Meeting",
		meta = (DeprecatedFunction, DeprecationMessage = "Use a pending Evidence Selection Request and TrySubmitClueForPendingDialogue."))
	EPFMeetingSubmitResult TrySubmitClueWithMatchingClaim(FGameplayTag ClueID);

	/** Submits a Clue through the pending Dialogue Evidence request. */
	UFUNCTION(BlueprintCallable, Category = "UI|Meeting")
	bool TrySubmitClueForPendingDialogue(FGameplayTag ClueID);

protected:
	/** Screen이 열릴 때 현재 Meeting 단서 목록으로 VerticalBox를 구성합니다. */
	virtual void NativeOnActivated() override;

private:
	/** 현재 제출 가능한 단서를 조회하고 지정된 버튼 WBP를 생성합니다. */
	void RefreshClueList();

	/** 생성된 단서 버튼 클릭을 Claim·Clue 연속 제출로 전달합니다. */
	void HandleClueClicked(const FGameplayTag& ClueID);

	/** 생성된 단서 버튼을 세로로 배치할 VerticalBox입니다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_ClueList;

	/** VerticalBox의 각 단서를 표시할 Entry WBP 클래스입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (DisplayName = "Clue Entry Class"))
	TSubclassOf<UPFClueSubmissionEntryWidget> m_ClueEntryClass;

	UPROPERTY(Transient)
	TObjectPtr<UPFDialogueGraphExecutor> m_DialogueExecutor;
};
