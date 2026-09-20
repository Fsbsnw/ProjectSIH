#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PFInvestigationMessages.generated.h"

USTRUCT()
struct PROJECT_SIH_API FPFInvestigationResult
{
	GENERATED_BODY()

	FGameplayTag m_CaseID;
	FGameplayTagContainer m_AcquiredClueIDs;

	bool IsValid() const
	{
		return m_CaseID.IsValid();
	}
};

USTRUCT()
struct PROJECT_SIH_API FPFInvestigationReadyMessage
{
	GENERATED_BODY()
};
