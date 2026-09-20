#include "PFClueDefinition.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/SIHGameplayTags.h"

FPrimaryAssetId UPFClueDefinition::GetPrimaryAssetId() const
{
	return MakePrimaryAssetID(m_ClueID);
}

FPrimaryAssetId UPFClueDefinition::MakePrimaryAssetID(
	const FGameplayTag& ClueID)
{
	if (!PFGameplayTagUtilities::IsValidChildTag(
		ClueID,
		SIHGameplayTags::ID_Clue))
	{
		PF_LOG(
			TEXT(
				"Tag is outside the Clue ID domain. "
				"ClueID=%s, RequiredRoot=%s"),
			*ClueID.ToString(),
			*SIHGameplayTags::ID_Clue.GetTag().ToString());
		return FPrimaryAssetId();
	}

	const FPrimaryAssetType AssetType(TEXT("Clue"));
	return FPrimaryAssetId(AssetType, ClueID.GetTagName());
}

bool UPFClueDefinition::HasValidCombinationRecipe() const
{
	if (!m_bIsCombinationResult
		|| m_CombinationSourceClueIDs.Num() < 2
		|| !PFGameplayTagUtilities::IsValidChildTag(
			m_ClueID,
			SIHGameplayTags::ID_Clue)
		|| m_CombinationSourceClueIDs.HasTagExact(m_ClueID))
	{
		return false;
	}

	for (const FGameplayTag& SourceClueID : m_CombinationSourceClueIDs)
	{
		if (!PFGameplayTagUtilities::IsValidChildTag(
			SourceClueID,
			SIHGameplayTags::ID_Clue))
		{
			return false;
		}
	}

	return true;
}

bool UPFClueDefinition::MatchesCombinationRecipe(
	const FGameplayTagContainer& SourceClueIDs) const
{
	return m_bIsCombinationResult
		&& m_CombinationSourceClueIDs.Num() == SourceClueIDs.Num()
		&& m_CombinationSourceClueIDs.HasAllExact(SourceClueIDs);
}
