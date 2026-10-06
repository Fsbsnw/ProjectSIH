#pragma once

#include "GameplayTagContainer.h"

namespace PFGameplayTagUtilities
{
	inline bool IsValidChildTag(
		const FGameplayTag& Tag,
		const FGameplayTag& RootTag)
	{
		return Tag.IsValid()
			&& RootTag.IsValid()
			&& Tag.MatchesTag(RootTag)
			&& !Tag.MatchesTagExact(RootTag);
	}
}
