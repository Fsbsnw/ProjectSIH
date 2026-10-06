#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "PFGameFlowSettings.generated.h"

class UWorld;

UCLASS(
	Config = Game,
	DefaultConfig,
	meta = (DisplayName = "PF Game Flow"))
class PROJECT_SIH_API UPFGameFlowSettings
	: public UDeveloperSettings
{
	GENERATED_BODY()

public:
	const TSoftObjectPtr<UWorld>&
		GetInvestigationMap() const
	{
		return m_InvestigationMap;
	}

	const TSoftObjectPtr<UWorld>& GetBattleMap() const
	{
		return m_BattleMap;
	}

	const TArray<FGameplayTag>& GetInitialCharacterIDs() const
	{
		return m_InitialCharacterIDs;
	}

private:
	UPROPERTY(
		Config,
		EditAnywhere,
		Category = "Maps",
		meta = (
			AllowPrivateAccess = "true",
			DisplayName = "Investigation Map"))
	TSoftObjectPtr<UWorld> m_InvestigationMap;

	UPROPERTY(
		Config,
		EditAnywhere,
		Category = "Maps",
		meta = (
			AllowPrivateAccess = "true",
			DisplayName = "Battle Map"))
	TSoftObjectPtr<UWorld> m_BattleMap;

	UPROPERTY(
		Config,
		EditAnywhere,
		Category = "New Game",
		meta = (
			AllowPrivateAccess = "true",
			Categories = "ID.Character",
			DisplayName = "Initial Characters"))
	TArray<FGameplayTag> m_InitialCharacterIDs;
};
