#include "PFItemDefinition.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/SIHGameplayTags.h"

FPrimaryAssetId UPFItemDefinition::GetPrimaryAssetId() const
{
	return MakePrimaryAssetID(m_DefinitionTag);
}

FPrimaryAssetId UPFItemDefinition::MakePrimaryAssetID(
	const FGameplayTag& ItemDefinitionID)
{
	if (!ItemDefinitionID.IsValid())
	{
		PF_LOG(
			TEXT("ItemDefinitionID is invalid. Value=%s"),
			*ItemDefinitionID.ToString());
		return FPrimaryAssetId();
	}

	const bool bIsItemDefinitionID =
		ItemDefinitionID.MatchesTag(SIHGameplayTags::ID_Item)
		&& ItemDefinitionID != SIHGameplayTags::ID_Item;

	if (!bIsItemDefinitionID)
	{
		PF_LOG(
			TEXT(
				"Tag is outside the Item ID domain. "
				"ItemDefinitionID=%s, RequiredRoot=%s"),
			*ItemDefinitionID.ToString(),
			*SIHGameplayTags::ID_Item.GetTag().ToString());
		return FPrimaryAssetId();
	}

	const FPrimaryAssetType AssetType(TEXT("Item"));

	return FPrimaryAssetId(
		AssetType,
		ItemDefinitionID.GetTagName());
}
