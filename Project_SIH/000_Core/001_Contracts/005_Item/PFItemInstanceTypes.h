#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"

#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/SIHGameplayTags.h"

#include "PFItemInstanceTypes.generated.h"

USTRUCT()
struct PROJECT_SIH_API FPFItemInstanceState
{
	GENERATED_BODY()
};

USTRUCT()
struct PROJECT_SIH_API FPFItemInstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag m_DefinitionID;

	UPROPERTY()
	TInstancedStruct<FPFItemInstanceState> m_InstanceState;

	bool IsValid() const
	{
		return PFGameplayTagUtilities::IsValidChildTag(
			m_DefinitionID,
			SIHGameplayTags::ID_Item);
	}
};
