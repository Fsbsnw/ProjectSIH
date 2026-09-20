#include "PFCaseDefinition.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/SIHGameplayTags.h"

FPrimaryAssetId UPFCaseDefinition::GetPrimaryAssetId() const
{
	return MakePrimaryAssetID(m_CaseID);
}

FPrimaryAssetId UPFCaseDefinition::MakePrimaryAssetID(
	const FGameplayTag& CaseID)
{
	if (!CaseID.IsValid())
	{
		PF_LOG(
			TEXT("CaseID is invalid. Value=%s"),
			*CaseID.ToString());
		return FPrimaryAssetId();
	}

	if (!PFGameplayTagUtilities::IsValidChildTag(
		CaseID,
		SIHGameplayTags::ID_Case))
	{
		PF_LOG(
			TEXT(
				"Tag is outside the Case ID domain. "
				"CaseID=%s, RequiredRoot=%s"),
			*CaseID.ToString(),
			*SIHGameplayTags::ID_Case.GetTag().ToString());
		return FPrimaryAssetId();
	}

	const FPrimaryAssetType AssetType(TEXT("Case"));
	return FPrimaryAssetId(AssetType, CaseID.GetTagName());
}
