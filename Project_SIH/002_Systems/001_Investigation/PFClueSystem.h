#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFClueTypes.h"
#include "UObject/Object.h"
#include "PFClueSystem.generated.h"

UCLASS()
class PROJECT_SIH_API UPFClueSystem : public UObject
{
	GENERATED_BODY()

public:
	void Init();
	void Deinit();

	bool TryResetForCase(const FGameplayTag& CaseID);

	EPFClueAcquireResult TryAcquireClue(const FGameplayTag& ClueID);
	FPFClueCombinationResult TryCombineClues(const FGameplayTagContainer& SourceClueIDs);

	bool HasClue(const FGameplayTag& ClueID) const;

	bool TryGetAcquiredClueIDs(FGameplayTagContainer& OutClueIDs) const;
	bool TryGetCombinationAttempts(TArray<FPFClueCombinationAttempt>& OutAttempts) const;

private:
	int32 FindCombinationAttemptIndex(const FGameplayTagContainer& SourceClueIDs) const;

	FGameplayTag m_ActiveCaseID;
	FGameplayTagContainer m_AcquiredClueIDs;
	TArray<FPFClueCombinationAttempt> m_CombinationAttempts;
};
