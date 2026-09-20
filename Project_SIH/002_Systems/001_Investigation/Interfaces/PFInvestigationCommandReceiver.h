#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFClueTypes.h"
#include "UObject/Interface.h"
#include "PFInvestigationCommandReceiver.generated.h"

UINTERFACE(MinimalAPI)
class UPFInvestigationCommandReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * 조사 진행에 필요한 외부 명령을 처리하기 위한 인터페이스입니다.
 */
class PROJECT_SIH_API IPFInvestigationCommandReceiver
{
	GENERATED_BODY()

public:
	virtual EPFClueAcquireResult TryAcquireClue(const FGameplayTag& ClueID) = 0;
	virtual FPFClueCombinationResult TryCombineClues(
		const FGameplayTagContainer& SourceClueIDs) = 0;
	virtual bool TryCompleteInvestigation(const FGameplayTag& CaseID) = 0;
	virtual bool TrySkipInvestigation(const FGameplayTag& CaseID) = 0;
};
