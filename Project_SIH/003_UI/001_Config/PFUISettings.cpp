#include "PFUISettings.h"

#include "GameFramework/GameModeBase.h"

const TSoftObjectPtr<UPFUIConfig>* UPFUISettings::FindUIConfigForGameMode(
	const UClass* GameModeClass) const
{
	if (!IsValid(GameModeClass) || !GameModeClass->IsChildOf<AGameModeBase>())
	{
		return nullptr;
	}

	for (const FPFGameModeUIConfig& Entry : m_GameModeUIConfigs)
	{
		UClass* ConfiguredGameModeClass = Entry.GameModeClass.LoadSynchronous();

		if (ConfiguredGameModeClass == GameModeClass && !Entry.UIConfig.IsNull())
		{
			return &Entry.UIConfig;
		}
	}

	return nullptr;
}
