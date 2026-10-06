#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PFUISettings.generated.h"

class AGameModeBase;
class UPFPrimaryGameLayout;
class UPFUIConfig;

/** GameMode 클래스와 해당 모드에서 사용할 UI Config 에셋의 Project Settings 항목입니다. */
USTRUCT()
struct FPFGameModeUIConfig
{
	GENERATED_BODY()

	/** UI Config 선택 기준이 되는 GameMode 클래스입니다. */
	UPROPERTY(Config, EditAnywhere, Category = "UI")
	TSoftClassPtr<AGameModeBase> GameModeClass;

	/** 해당 GameMode가 사용할 UI Config 에셋입니다. */
	UPROPERTY(Config, EditAnywhere, Category = "UI")
	TSoftObjectPtr<UPFUIConfig> UIConfig;
};

/**
 * Project Settings에서 로컬 플레이어의 시작 Layout과 GameMode별 UI Config를 지정합니다.
 * 런타임에는 UPFUIManagerSubsystem만 이 설정을 읽습니다.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "PF UI Settings"))
class PROJECT_SIH_API UPFUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Project Settings의 Game 카테고리에 이 설정을 표시합니다. */
	virtual FName GetCategoryName() const override
	{
		return TEXT("Game");
	}

	/** 모든 GameMode가 공유하는 최상위 Layout 클래스를 반환합니다. */
	const TSoftClassPtr<UPFPrimaryGameLayout>& GetPrimaryGameLayoutClass() const
	{
		return m_PrimaryGameLayoutClass;
	}

	/** 모든 GameMode에서 공통으로 조회할 UI Config를 반환합니다. */
	const TSoftObjectPtr<UPFUIConfig>& GetGlobalUIConfig() const
	{
		return m_GlobalUIConfig;
	}

	/** 정확히 일치하는 GameMode 클래스에 설정된 UI Config를 찾습니다. */
	const TSoftObjectPtr<UPFUIConfig>* FindUIConfigForGameMode(const UClass* GameModeClass) const;

private:
	/** 모든 게임 모드에서 공통으로 사용하는 최상위 UI 레이아웃 클래스입니다. */
	UPROPERTY(Config, EditAnywhere, Category = "Layout", meta = (DisplayName = "Primary Game Layout Class"))
	TSoftClassPtr<UPFPrimaryGameLayout> m_PrimaryGameLayoutClass;

	/** 모든 게임 모드에서 공통으로 사용할 UIConfig입니다. */
	UPROPERTY(Config, EditAnywhere, Category = "Config", meta = (DisplayName = "Global UI Config"))
	TSoftObjectPtr<UPFUIConfig> m_GlobalUIConfig;

	/** 게임 모드별로 사용할 UIConfig를 매핑합니다. */
	UPROPERTY(Config, EditAnywhere, Category = "Config", meta = (DisplayName = "Game Mode UI Configs"))
	TArray<FPFGameModeUIConfig> m_GameModeUIConfigs;
};
