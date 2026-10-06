#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/000_Core/001_Contracts/006_Dialogue/PFDialogueContracts.h"
#include "UObject/Object.h"
#include "PFDialogueGraphExecutor.generated.h"

class UPFDialogueGraphAsset;
struct FPFDialogueRuntimeNode;

UENUM(BlueprintType)
enum class EPFDialogueExecutionState : uint8
{
	Idle,
	WaitingForContinue,
	WaitingForInlineChoice,
	WaitingForExternalResponse,
	Completed,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPFDialogueLineRequested,
	const FPFDialogueLinePresentation&,
	Presentation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPFDialogueInlineChoiceRequested,
	const FPFDialogueInlineChoicePresentation&,
	Presentation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPFDialogueExternalSelectionRequested,
	const FPFDialogueExternalSelectionRequest&,
	Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPFDialogueExecutionCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPFDialogueExecutionFailed);

/**
 * Minimal runtime cursor for a compiled Dialogue Graph asset.
 * It owns only transient execution/request state; domain systems remain their own SoT.
 */
UCLASS(BlueprintType)
class PROJECT_SIH_API UPFDialogueGraphExecutor final : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool Start(UPFDialogueGraphAsset* DialogueAsset);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool ContinueDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool SelectInlineChoice(int32 ChoiceIndex);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool SubmitExternalResponse(const FPFDialogueExternalSelectionResponse& Response);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void Cancel();

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	EPFDialogueExecutionState GetExecutionState() const { return m_ExecutionState; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FPFDialogueLinePresentation GetCurrentLine() const { return m_CurrentLine; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FPFDialogueInlineChoicePresentation GetCurrentInlineChoice() const
	{
		return m_CurrentInlineChoice;
	}

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FPFDialogueExternalSelectionRequest GetPendingExternalRequest() const
	{
		return m_PendingExternalRequest;
	}

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FGameplayTag GetLastSelectedContentID() const { return m_LastSelectedContentID; }

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FPFDialogueLineRequested OnLineRequested;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FPFDialogueInlineChoiceRequested OnInlineChoiceRequested;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FPFDialogueExternalSelectionRequested OnExternalSelectionRequested;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FPFDialogueExecutionCompleted OnDialogueCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FPFDialogueExecutionFailed OnDialogueFailed;

private:
	bool EnterNode(const FGuid& NodeID);
	bool AdvanceFromCurrentNode(FName OutputName);
	bool BuildLinePresentation(const FPFDialogueRuntimeNode& Node);
	bool BuildInlineChoicePresentation(const FPFDialogueRuntimeNode& Node);
	bool BuildExternalRequest(const FPFDialogueRuntimeNode& Node);
	void SetFailed();

	UPROPERTY(Transient)
	TObjectPtr<UPFDialogueGraphAsset> m_DialogueAsset;

	FGuid m_CurrentNodeID;
	EPFDialogueExecutionState m_ExecutionState = EPFDialogueExecutionState::Idle;
	FPFDialogueLinePresentation m_CurrentLine;
	FPFDialogueInlineChoicePresentation m_CurrentInlineChoice;
	FPFDialogueExternalSelectionRequest m_PendingExternalRequest;
	FGameplayTag m_ActiveClaimID;
	FGameplayTag m_LastSelectedContentID;
};
