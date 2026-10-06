// Fill out your copyright notice in the Description page of Project Settings.


#include "PFCharacterDefinition.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/SIHGameplayTags.h"


FPrimaryAssetId UPFCharacterDefinition::GetPrimaryAssetId() const
{
	FPrimaryAssetId CharacterID = MakePrimaryAssetID(m_DefinitionTag);
	return CharacterID;
}

FPrimaryAssetId UPFCharacterDefinition::MakePrimaryAssetID(const FGameplayTag& CharacterID)
{
	if (!CharacterID.IsValid())
	{
		PF_LOG(
			TEXT("CharacterID is invalid. Value=%s"),
			*CharacterID.ToString());
		return FPrimaryAssetId();
	}

	if (!PFGameplayTagUtilities::IsValidChildTag(
		CharacterID,
		SIHGameplayTags::ID_Character))
	{
		PF_LOG(
			TEXT(
				"Tag is outside the Character ID domain. "
				"CharacterID=%s, RequiredRoot=%s"),
			*CharacterID.ToString(),
			*SIHGameplayTags::ID_Character.GetTag().ToString());
		return FPrimaryAssetId();
	}

	FPrimaryAssetType AssetType = FPrimaryAssetType(TEXT("Character"));

	return FPrimaryAssetId(AssetType, CharacterID.GetTagName());
}

const FPFCombatProfile&
UPFCharacterDefinition::GetDefaultCombatProfile() const
{
	return m_DefaultCombatProfile;
}

const UCurveTable* UPFCharacterDefinition::GetLevelBonusTable() const
{
	return m_LevelBonusTable;
}

#if WITH_EDITOR
bool UPFCharacterDefinition::SetDefinitionTagForEditorImport(
	const FGameplayTag& InDefinitionTag)
{
	if (!MakePrimaryAssetID(InDefinitionTag).IsValid())
	{
		PF_LOG(
			TEXT(
				"Rejected CharacterID during Editor import. "
				"CharacterID=%s"),
			*InDefinitionTag.ToString());
		return false;
	}

	m_DefinitionTag = InDefinitionTag;
	return true;
}

void UPFCharacterDefinition::SetPresentationDataForEditorImport(
	const FText& InDisplayName,
	const TSoftObjectPtr<UTexture2D>& InIcon)
{
	m_DisplayName = InDisplayName;
	m_Icon = InIcon;
}
#endif
