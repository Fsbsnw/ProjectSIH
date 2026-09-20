#pragma once

#include "CoreMinimal.h"
#include "PFMeetingTypes.generated.h"

UENUM()
enum class EPFMeetingClaimSelectResult : uint8
{
	None,
	Selected,
	InactiveMeeting,
	InvalidStep,
	InvalidClaimID,
};

UENUM()
enum class EPFMeetingSubmitResult : uint8
{
	None,
	Correct,
	Incorrect,
	InactiveMeeting,
	InvalidStep,
	InvalidClueID,
	ClueNotOwned,
	ClueNotMeetingUsable,
	NotReady,
};
