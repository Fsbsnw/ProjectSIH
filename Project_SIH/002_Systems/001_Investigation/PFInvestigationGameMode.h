#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/PFInvestigationCommandReceiver.h"
#include "Interfaces/PFInvestigationEntryReceiver.h"
#include "Interfaces/PFMeetingCommandReceiver.h"
#include "Interfaces/PFMeetingEntryReceiver.h"
#include "PFInvestigationGameMode.generated.h"

struct FPFInvestigationResult;
class UPFMeetingSystem;
class UPFInvestigationSystem;
class UPFDialogueGraphExecutor;
UCLASS()
class PROJECT_SIH_API APFInvestigationGameMode : public AGameModeBase, public IPFInvestigationEntryReceiver, public IPFInvestigationCommandReceiver, public IPFMeetingEntryReceiver, public IPFMeetingCommandReceiver
{
public:
	APFInvestigationGameMode();

	virtual EPFPhaseStartResult StartMeeting(const FPFMeetingEntryContext& Context) override;

	virtual EPFPhaseStartResult StartInvestigation(const FPFInvestigationEntryContext& Context) override;

	virtual EPFClueAcquireResult TryAcquireClue(const FGameplayTag& ClueID) override;
	virtual FPFClueCombinationResult TryCombineClues(
		const FGameplayTagContainer& SourceClueIDs) override;
	virtual bool TryCompleteInvestigation(const FGameplayTag& CaseID) override;
	virtual bool TrySkipInvestigation(const FGameplayTag& CaseID) override;
	virtual bool TryGetAcquiredClueIDs(FGameplayTagContainer& OutClueIDs) const override;
	virtual EPFMeetingClaimSelectResult TrySelectClaim(const FGameplayTag& ClaimID) override;
	virtual EPFMeetingSubmitResult TrySubmitClue(const FGameplayTag& ClueID) override;
	virtual bool TryCancelSelection() override;
	virtual bool TryCompleteMeeting(const FGameplayTag& CaseID) override;

	/** Validates a pending Claim request, updates MeetingSystem, and resumes the graph. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Meeting")
	bool SubmitDialogueClaimSelection(
		UPFDialogueGraphExecutor* Executor,
		FGuid RequestID,
		FGameplayTag ClaimID);

	/** Submits one owned Clue for the pending Claim and resumes through Correct/Incorrect. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Meeting")
	bool SubmitDialogueEvidenceSelection(
		UPFDialogueGraphExecutor* Executor,
		FGuid RequestID,
		FGameplayTag ClueID);

	/** Cancels the pending external selection and restores Meeting to Claim selection. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Meeting")
	bool CancelDialogueExternalSelection(
		UPFDialogueGraphExecutor* Executor,
		FGuid RequestID);

private:
	GENERATED_BODY()

protected:
	virtual void InitGame(
		const FString& MapName,
		const FString& Options,
		FString& ErrorMessage) override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SendInvestigationReadyMessage();
private:
	UPROPERTY(Transient)
	TObjectPtr<UPFInvestigationSystem> m_InvestigationSystem;

	UPROPERTY(Transient)
	TObjectPtr<UPFMeetingSystem> m_MeetingSystem;
};
