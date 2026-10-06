#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/000_Core/001_Contracts/000_Flow/PFGameFlowTypes.h"
#include "Project_SIH/003_UI/000_Foundation/PFActivatableWidget.h"
#include "PFPhaseExitConfirmWidget.generated.h"

class UCommonButtonBase;
class UCommonTextBlock;
class UWidget;

/**
 * 현재 Investigation 또는 Meeting 종료를 확인하고 해당 단계의 종료 명령을 전달하는 전용 Modal입니다.
 * Case 상태는 소유하지 않고 GameFlow의 현재 상태를 활성화·확인 시점마다 검증합니다.
 */
UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFPhaseExitConfirmWidget : public UPFActivatableWidget
{
	GENERATED_BODY()

protected:
	/** 선택 가능한 버튼과 C++ 처리 함수를 연결합니다. */
	virtual void NativeOnInitialized() override;

	/** 현재 Case/Phase를 확인하고 종료 요청 Context와 문구를 준비합니다. */
	virtual void NativeOnActivated() override;

	/** 닫힌 Modal의 임시 Case/Phase Context를 정리합니다. */
	virtual void NativeOnDeactivated() override;

	/** 키보드·게임패드 포커스의 기본 대상을 확인 버튼으로 지정합니다. */
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	/** CommonUI Back 입력을 취소와 동일하게 처리합니다. */
	virtual bool NativeOnHandleBackAction() override;

private:
	/** 현재 종료 가능한 Case/Phase를 보관하고 확인 문구를 갱신합니다. */
	bool PreparePhaseExit();

	/** 보관한 Case/Phase가 여전히 유효할 때 대응하는 종료 명령을 전달합니다. */
	bool TryCompletePhaseExit();

	/** 완료된 Investigation 또는 Meeting 결과를 테스트 로그로 출력합니다. */
	void LogCompletionResult() const;

	/** 닫히거나 처리된 종료 요청의 최소 Context를 초기화합니다. */
	void ResetPendingPhaseExit();

	/** 현재 Phase의 종료 명령을 전달하고 Modal을 닫습니다. */
	void HandleConfirmClicked();

	/** 종료 요청을 취소하고 Modal을 닫습니다. */
	void HandleCancelClicked();

	/** 확인 문구를 표시하는 선택적 Text Widget입니다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Prompt;

	/** Phase 종료를 확정하는 선택적 Common Button입니다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Confirm;

	/** 요청을 취소하는 선택적 Common Button입니다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Cancel;

	/** Investigation 종료 확인창에 표시할 문구입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "UI|Phase Exit", meta = (DisplayName = "Investigation Exit Prompt"))
	FText m_InvestigationExitPrompt;

	/** Meeting 종료 확인창에 표시할 문구입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "UI|Phase Exit", meta = (DisplayName = "Meeting Exit Prompt"))
	FText m_MeetingExitPrompt;

	/** Modal 활성화 시 보관한 Case ID입니다. */
	FGameplayTag m_PendingCaseID;

	/** Modal 활성화 시 보관한 Case Phase입니다. */
	EPFCasePhase m_PendingPhase = EPFCasePhase::None;

	/** 한 번의 활성화에서 Confirm 또는 Cancel이 중복 처리되는 것을 방지합니다. */
	bool m_bResponseSent = false;
};
