#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/000_Core/001_Contracts/000_Flow/PFGameFlowTypes.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFMeetingTypes.h"
#include "UObject/Object.h"
#include "PFMeetingSystem.generated.h"

struct FPFMeetingResult;

UCLASS()
class PROJECT_SIH_API UPFMeetingSystem : public UObject
{
	GENERATED_BODY()

public:
	void Init();
	void Deinit();
	EPFPhaseStartResult StartMeeting(const FPFMeetingEntryContext& Context);
	bool TryGetAcquiredClueIDs(FGameplayTagContainer& OutClueIDs) const;
	EPFMeetingClaimSelectResult TrySelectClaim(const FGameplayTag& ClaimID);
	EPFMeetingSubmitResult TrySubmitClue(const FGameplayTag& ClueID);
	bool TryCompleteMeeting(const FGameplayTag& CaseID);

private:
	enum class EPFMeetingInputStep : uint8
	{
		Inactive,
		SelectingClaim,
		SelectingClue,
	};

	void ResetState();
	void SendMeetingCompletedMessage(const FPFMeetingResult& Result);

	FGameplayTag m_CaseID;
	FGameplayTagContainer m_AcquiredClueIDs;
	FGameplayTagContainer m_RevealedWeaknessIDs;
	FGameplayTag m_SelectedClaimID;
	EPFMeetingInputStep m_InputStep = EPFMeetingInputStep::Inactive;
};
