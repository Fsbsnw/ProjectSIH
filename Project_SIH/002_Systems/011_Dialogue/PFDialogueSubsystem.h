#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Project_SIH/000_Core/001_Contracts/006_Dialogue/PFDialogueTypes.h"
#include "PFDialogueSubsystem.generated.h"

class UPFDialogueDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPFDialogueLinePresented,
	const FPFDialogueLinePresentation&,
	Presentation);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPFDialogueChoicesPresented,
	const FPFDialogueChoicePresentation&,
	Presentation);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPFDialogueActionRequested,
	const FPFDialogueActionPresentation&,
	Presentation);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPFDialogueCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPFDialogueFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPFDialogueCancelled);

/**
 * 현재 World의 대화 Definition을 직접 실행합니다.
 * UI에는 표시 데이터를 전달하고 비동기 Action은 외부 실행 주체에 위임합니다.
 */
UCLASS()
class PROJECT_SIH_API UPFDialogueSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	EPFDialogueInputResult StartDialogue(UPFDialogueDefinition* DialogueDefinition);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	EPFDialogueInputResult ContinueDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	EPFDialogueInputResult SelectChoice(FName ChoiceID);

	/** Action 요청을 받은 실행 주체가 결과 ID와 요청 당시 Node ID를 돌려줍니다. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	EPFDialogueInputResult CompleteAction(FName ActionNodeID, FName ResultID);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	EPFDialogueInputResult CancelDialogue();

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsDialogueActive() const;

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool HasCurrentLine() const;

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool HasCurrentChoices() const;

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsWaitingForAction() const;

	const FPFDialogueLinePresentation& GetCurrentLinePresentation() const;
	const FPFDialogueChoicePresentation& GetCurrentChoicePresentation() const;
	const FPFDialogueActionPresentation& GetCurrentActionPresentation() const;

	FPFDialogueLinePresented& GetLinePresentedDelegate();
	FPFDialogueChoicesPresented& GetChoicesPresentedDelegate();
	FPFDialogueActionRequested& GetActionRequestedDelegate();
	FPFDialogueCompleted& GetCompletedDelegate();
	FPFDialogueFailed& GetFailedDelegate();
	FPFDialogueCancelled& GetCancelledDelegate();

private:
	enum class EPFDialogueStep : uint8
	{
		None,
		Line,
		Choices,
		Action,
		TransitionPending,
	};

	bool EnterNode(FName NodeID);
	bool PresentCurrentLine();
	bool PresentCurrentChoices();
	bool RequestCurrentAction();

	const FPFDialogueRuntimeNode* GetCurrentNode() const;

	void ArchiveCurrentChoices();
	void CompleteActiveDialogue();
	void FailActiveDialogue();
	void ClearRuntimeState();
	bool IsInputReentrant() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPFDialogueDefinition> m_ActiveDefinition;

	FName m_CurrentNodeID;
	FName m_LastCompletedActionNodeID;
	int32 m_CurrentLineIndex = INDEX_NONE;

	FPFDialogueLinePresentation m_CurrentLinePresentation;
	FPFDialogueChoicePresentation m_CurrentChoicePresentation;
	FPFDialogueActionPresentation m_CurrentActionPresentation;

	TSet<FName> m_PreviouslyPresentedChoiceIDs;

	EPFDialogueStep m_CurrentStep = EPFDialogueStep::None;
	bool m_bDialogueActive = false;
	bool m_bProcessingDialogue = false;
	bool m_bBroadcastingDelegate = false;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue",
		meta = (DisplayName = "On Line Presented"))
	FPFDialogueLinePresented m_OnLinePresented;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue",
		meta = (DisplayName = "On Choices Presented"))
	FPFDialogueChoicesPresented m_OnChoicesPresented;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue",
		meta = (DisplayName = "On Action Requested"))
	FPFDialogueActionRequested m_OnActionRequested;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue",
		meta = (DisplayName = "On Completed"))
	FPFDialogueCompleted m_OnCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue",
		meta = (DisplayName = "On Failed"))
	FPFDialogueFailed m_OnFailed;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue",
		meta = (DisplayName = "On Cancelled"))
	FPFDialogueCancelled m_OnCancelled;
};
