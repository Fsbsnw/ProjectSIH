#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFMeetingTypes.h"
#include "UObject/Interface.h"
#include "PFMeetingCommandReceiver.generated.h"

UINTERFACE(MinimalAPI)
class UPFMeetingCommandReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * 회의 진행 중 주장 선택과 단서 제출 명령을 처리하기 위한 인터페이스입니다.
 */
class PROJECT_SIH_API IPFMeetingCommandReceiver
{
	GENERATED_BODY()

public:
	virtual bool TryGetAcquiredClueIDs(FGameplayTagContainer& OutClueIDs) const = 0;
	virtual EPFMeetingClaimSelectResult TrySelectClaim(const FGameplayTag& ClaimID) = 0;
	virtual EPFMeetingSubmitResult TrySubmitClue(const FGameplayTag& ClueID) = 0;
	virtual bool TryCompleteMeeting(const FGameplayTag& CaseID) = 0;
};
