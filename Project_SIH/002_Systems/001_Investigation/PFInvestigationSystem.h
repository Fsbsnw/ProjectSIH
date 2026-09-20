#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/000_Core/001_Contracts/000_Flow/PFGameFlowTypes.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFClueTypes.h"
#include "UObject/Object.h"
#include "PFInvestigationSystem.generated.h"


class UPFClueSystem;
struct FPFInvestigationResult;

UCLASS()
class PROJECT_SIH_API UPFInvestigationSystem : public UObject
{
	GENERATED_BODY()

public:
	void Init();
	void Deinit();

	EPFPhaseStartResult StartInvestigation(const FPFInvestigationEntryContext& Context);
	bool TryCompleteInvestigation(const FGameplayTag& CaseID);
	bool TrySkipInvestigation(const FGameplayTag& CaseID);

	EPFClueAcquireResult TryAcquireClue(const FGameplayTag& ClueID);
	FPFClueCombinationResult TryCombineClues(const FGameplayTagContainer& SourceClueIDs);

	bool HasClue(const FGameplayTag& ClueID) const;

	bool TryGetAcquiredClueIDs(FGameplayTagContainer& OutClueIDs) const;

private:
	bool TryFinishInvestigation(
		const FGameplayTag& CaseID,
		const FGameplayTag& ResultChannel);
	void SendInvestigationResult(
		const FGameplayTag& Channel,
		const FPFInvestigationResult& Result);

private:
	UPROPERTY(Transient)
	TObjectPtr<UPFClueSystem> m_ClueSystem;

	FGameplayTag m_ActiveCaseId;
};
