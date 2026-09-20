#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PFGameFlowSettings.generated.h"

class UWorld;

UCLASS(
	Config = Game,
	DefaultConfig,
	meta = (DisplayName = "Game Flow"))
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
};
