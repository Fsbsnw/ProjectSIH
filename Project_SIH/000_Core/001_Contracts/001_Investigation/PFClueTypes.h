#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PFClueTypes.generated.h"

enum class EPFClueAcquireResult : uint8
{
	None,
	Acquired,
	AlreadyOwned,
	InvalidClueID,
	DefinitionUnavailable,
	InactiveCase,
};

enum class EPFClueCombineResult : uint8
{
	None,
	Combined,
	ResultAlreadyOwned,
	InvalidSourceClueID,
	InvalidSourceClueCount,
	SourceClueNotOwned,
	SourceDefinitionUnavailable,
	SourceClueNotCombinable,
	RecipeUnavailable,
	AlreadyReviewed,
	InvalidResultDefinition,
	InactiveCase,
};

/**
 * 단서 조합 시도의 결과를 표현합니다.
 */
UENUM()
enum class EPFClueCombinationAttemptResult : uint8
{
	None,
	Success,
	Failed,
};

/**
 * 단서 조합 시도에 사용된 단서들과 조합 결과를 기록합니다.
 */
USTRUCT()
struct PROJECT_SIH_API FPFClueCombinationAttempt
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTagContainer m_SourceClueIDs;

	UPROPERTY()
	EPFClueCombinationAttemptResult m_Result = EPFClueCombinationAttemptResult::None;

	UPROPERTY()
	FGameplayTag m_ResultClueID;

	// 단서 개수와 구성이 동일한 조합인지 확인합니다.
	bool MatchesSourceClueIDs(const FGameplayTagContainer& SourceClueIDs) const
	{
		return m_SourceClueIDs.Num() == SourceClueIDs.Num()
			&& m_SourceClueIDs.HasAllExact(SourceClueIDs);
	}
};

struct PROJECT_SIH_API FPFClueCombinationResult
{
	EPFClueCombineResult m_Result = EPFClueCombineResult::None;
	FGameplayTag m_ResultClueID;
};
