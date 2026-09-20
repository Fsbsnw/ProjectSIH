// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "GameplayTagContainer.h"
#include "PFAssetManager.generated.h"

class UPFCaseDefinition;
class UPFClueDefinition;
/**
 *
 */
class UPFCharacterDefinition;
class UPFItemDefinition;

UCLASS()
class PROJECT_SIH_API UPFAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UPFAssetManager& Get();

	const UPFCharacterDefinition* GetCharacterDefinition(const FGameplayTag& CharacterID);
	const UPFCaseDefinition* GetCaseDefinition(const FGameplayTag& CaseID);
	const UPFClueDefinition* GetClueDefinition(const FGameplayTag& ClueID);
	const UPFClueDefinition* GetClueCombinationDefinition(
		const FGameplayTagContainer& SourceClueIDs);
	const UPFItemDefinition* GetItemDefinition(const FGameplayTag& ItemDefinitionID);

private:
	UObject* GetOrLoadPrimaryAssetObjectSync(const FPrimaryAssetId& AssetID);
};
