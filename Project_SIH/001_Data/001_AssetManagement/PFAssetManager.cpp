// Fill out your copyright notice in the Description page of Project Settings.


#include "PFAssetManager.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/001_Data/000_Definitions/PFCaseDefinition.h"
#include "Project_SIH/001_Data/000_Definitions/PFCharacterDefinition.h"
#include "Project_SIH/001_Data/000_Definitions/PFClueDefinition.h"
#include "Project_SIH/001_Data/000_Definitions/PFItemDefinition.h"
#include "Project_SIH/SIHGameplayTags.h"


UPFAssetManager& UPFAssetManager::Get()
{
	UAssetManager& BaseAssetManager = UAssetManager::Get();
	UPFAssetManager* PFAssetManager = CastChecked<UPFAssetManager>(&BaseAssetManager);
	return *PFAssetManager;
}

UObject* UPFAssetManager::GetOrLoadPrimaryAssetObjectSync(
	const FPrimaryAssetId& AssetID)
{
	if (!AssetID.IsValid())
	{
		PF_LOG(
			TEXT("PrimaryAssetID is invalid. Value=%s"),
			*AssetID.ToString());
		return nullptr;
	}

	if (UObject* LoadedObject = GetPrimaryAssetObject(AssetID))
	{
		return LoadedObject;
	}

	const FSoftObjectPath AssetPath = GetPrimaryAssetPath(AssetID);

	if (!AssetPath.IsValid())
	{
		PF_LOG(
			TEXT("Primary Asset is not registered. AssetID=%s"),
			*AssetID.ToString());
		return nullptr;
	}

	TSharedPtr<FStreamableHandle> LoadHandle =
		LoadPrimaryAsset(AssetID);

	if (!LoadHandle.IsValid())
	{
		if (UObject* LoadedObject = GetPrimaryAssetObject(AssetID))
		{
			return LoadedObject;
		}

		PF_LOG(
			TEXT("Failed to create load handle. AssetID=%s"),
			*AssetID.ToString());
		return nullptr;
	}

	LoadHandle->WaitUntilComplete();

	UObject* LoadedObject = GetPrimaryAssetObject(AssetID);

	if (!LoadedObject)
	{
		PF_LOG(
			TEXT("Failed to load Primary Asset. AssetID=%s"),
			*AssetID.ToString());
		return nullptr;
	}

	return LoadedObject;
}

const UPFCharacterDefinition* UPFAssetManager::GetCharacterDefinition(
	const FGameplayTag& CharacterID)
{
	const FPrimaryAssetId AssetID =
		UPFCharacterDefinition::MakePrimaryAssetID(CharacterID);

	if (!AssetID.IsValid())
	{
		PF_LOG(
			TEXT("Invalid CharacterID: %s"),
			*CharacterID.ToString());
		return nullptr;
	}

	UObject* LoadedObject =
		GetOrLoadPrimaryAssetObjectSync(AssetID);

	if (!LoadedObject)
	{
		return nullptr;
	}

	const UPFCharacterDefinition* CharacterDefinition =
		Cast<UPFCharacterDefinition>(LoadedObject);

	if (!CharacterDefinition)
	{
		PF_LOG(
			TEXT(
				"Loaded Primary Asset has unexpected class. "
				"AssetID=%s, ExpectedClass=%s, ActualClass=%s"),
			*AssetID.ToString(),
			*UPFCharacterDefinition::StaticClass()->GetName(),
			*LoadedObject->GetClass()->GetName());
		return nullptr;
	}

	return CharacterDefinition;
}

const UPFCaseDefinition* UPFAssetManager::GetCaseDefinition(
	const FGameplayTag& CaseID)
{
	const FPrimaryAssetId AssetID =
		UPFCaseDefinition::MakePrimaryAssetID(CaseID);

	if (!AssetID.IsValid())
	{
		PF_LOG(
			TEXT("Invalid CaseID: %s"),
			*CaseID.ToString());
		return nullptr;
	}

	UObject* LoadedObject =
		GetOrLoadPrimaryAssetObjectSync(AssetID);

	if (!LoadedObject)
	{
		return nullptr;
	}

	const UPFCaseDefinition* CaseDefinition =
		Cast<UPFCaseDefinition>(LoadedObject);

	if (!CaseDefinition)
	{
		PF_LOG(
			TEXT(
				"Loaded Primary Asset has unexpected class. "
				"AssetID=%s, ExpectedClass=%s, ActualClass=%s"),
			*AssetID.ToString(),
			*UPFCaseDefinition::StaticClass()->GetName(),
			*LoadedObject->GetClass()->GetName());
		return nullptr;
	}

	return CaseDefinition;
}

const UPFClueDefinition* UPFAssetManager::GetClueDefinition(
	const FGameplayTag& ClueID)
{
	const FPrimaryAssetId AssetID =
		UPFClueDefinition::MakePrimaryAssetID(ClueID);

	if (!AssetID.IsValid())
	{
		PF_LOG(
			TEXT("Invalid ClueID: %s"),
			*ClueID.ToString());
		return nullptr;
	}

	UObject* LoadedObject =
		GetOrLoadPrimaryAssetObjectSync(AssetID);

	if (!LoadedObject)
	{
		return nullptr;
	}

	const UPFClueDefinition* ClueDefinition =
		Cast<UPFClueDefinition>(LoadedObject);

	if (!ClueDefinition)
	{
		PF_LOG(
			TEXT(
				"Loaded Primary Asset has unexpected class. "
				"AssetID=%s, ExpectedClass=%s, ActualClass=%s"),
			*AssetID.ToString(),
			*UPFClueDefinition::StaticClass()->GetName(),
			*LoadedObject->GetClass()->GetName());
		return nullptr;
	}

	return ClueDefinition;
}

const UPFClueDefinition* UPFAssetManager::GetClueCombinationDefinition(
	const FGameplayTagContainer& SourceClueIDs)
{
	// 조합에 사용되는 단서가 2개 미만이면 유효한 조합식으로 처리하지 않습니다.
	if (SourceClueIDs.Num() < 2)
	{
		PF_LOG(
			TEXT(
				"A Clue combination requires at least two source Clues. "
				"SourceClueCount=%d"),
			SourceClueIDs.Num());
		return nullptr;
	}

	// 입력된 단서 ID 정보들이 유효한지 판단합니다.
	for (const FGameplayTag& SourceClueID : SourceClueIDs)
	{
		if (!PFGameplayTagUtilities::IsValidChildTag(
				SourceClueID,
				SIHGameplayTags::ID_Clue))
		{
			PF_LOG(
				TEXT("Invalid SourceClueID: %s"),
				*SourceClueID.ToString());
			return nullptr;
		}
	}

	TArray<FPrimaryAssetId> ClueAssetIDs;
	if (!GetPrimaryAssetIdList(FPrimaryAssetType(TEXT("Clue")), ClueAssetIDs))
	{
		PF_LOG(TEXT("There are no registered Clue Primary Assets."));
		return nullptr;
	}

	const UPFClueDefinition* MatchingDefinition = nullptr;
	for (const FPrimaryAssetId& ClueAssetID : ClueAssetIDs)
	{
		UObject* LoadedObject =
			GetOrLoadPrimaryAssetObjectSync(ClueAssetID);

		if (!LoadedObject)
		{
			return nullptr;
		}

		const UPFClueDefinition* ClueDefinition =
			Cast<UPFClueDefinition>(LoadedObject);

		if (!ClueDefinition)
		{
			PF_LOG(
				TEXT(
					"Loaded Primary Asset has unexpected class. "
					"AssetID=%s, ExpectedClass=%s, ActualClass=%s"),
				*ClueAssetID.ToString(),
				*UPFClueDefinition::StaticClass()->GetName(),
				*LoadedObject->GetClass()->GetName());
			return nullptr;
		}

		// 입력 받은 단서 정보들과 일치하는 단서 조합 레시피 정보를 확인합니다.
		if (!ClueDefinition->MatchesCombinationRecipe(SourceClueIDs))
		{
			continue;
		}

		// 동일한 단서 조합 레시피를 가진 Definition이 여러 개 존재하면 중복으로 처리합니다.
		if (MatchingDefinition)
		{
			PF_LOG(
				TEXT(
					"Multiple Clue definitions contain the same combination recipe. "
					"SourceClueCount=%d"),
				SourceClueIDs.Num());
			return nullptr;
		}

		MatchingDefinition = ClueDefinition;
	}

	return MatchingDefinition;
}

const UPFItemDefinition* UPFAssetManager::GetItemDefinition(
	const FGameplayTag& ItemDefinitionID)
{
	const FPrimaryAssetId AssetID =
		UPFItemDefinition::MakePrimaryAssetID(ItemDefinitionID);

	if (!AssetID.IsValid())
	{
		PF_LOG(
			TEXT("Invalid ItemDefinitionID: %s"),
			*ItemDefinitionID.ToString());
		return nullptr;
	}

	UObject* LoadedObject =
		GetOrLoadPrimaryAssetObjectSync(AssetID);

	if (!LoadedObject)
	{
		return nullptr;
	}

	const UPFItemDefinition* ItemDefinition =
		Cast<UPFItemDefinition>(LoadedObject);

	if (!ItemDefinition)
	{
		PF_LOG(
			TEXT(
				"Loaded Primary Asset has unexpected class. "
				"AssetID=%s, ExpectedClass=%s, ActualClass=%s"),
			*AssetID.ToString(),
			*UPFItemDefinition::StaticClass()->GetName(),
			*LoadedObject->GetClass()->GetName());
		return nullptr;
	}

	return ItemDefinition;
}
