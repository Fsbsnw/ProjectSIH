#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct FPFPartyData
{
	static constexpr int32 RequiredMemberCount = 4;

	FGameplayTagContainer m_CharacterIDs;

	bool IsValid() const
	{
		if (m_CharacterIDs.Num() != RequiredMemberCount)
		{
			return false;
		}

		for (const FGameplayTag& CharacterID : m_CharacterIDs)
		{
			if (!CharacterID.IsValid())
			{
				return false;
			}
		}

		return true;
	}
};
