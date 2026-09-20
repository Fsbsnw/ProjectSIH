#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PFMeetingMessages.generated.h"

USTRUCT()
struct PROJECT_SIH_API FPFMeetingResult
{
	GENERATED_BODY()

	FGameplayTag m_CaseID;
	FGameplayTagContainer m_RevealedWeaknessIDs;

	bool IsValid() const
	{
		return m_CaseID.IsValid();
	}
};
