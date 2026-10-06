#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/003_UI/000_Foundation/PFGameUIRootBase.h"
#include "PFUIConfig.generated.h"

class UPFActivatableWidget;

/**
 * 하나의 UI ID에 대응하는 위젯 클래스와 선택적 표시 입력을 정의합니다.
 * Open Input Action이 비어 있으면 버튼·상호작용 등 외부 ShowWidget 요청으로만 표시됩니다.
 */
USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFUIWidgetDefinition
{
	GENERATED_BODY()

	/** UIManager가 해당 ID를 표시할 때 생성할 CommonUI 위젯 클래스입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (DisplayName = "Widget Class"))
	TSoftClassPtr<UPFActivatableWidget> m_WidgetClass;

	/** Root가 이 위젯을 표시하도록 등록할 선택적 CommonUI Action입니다. */
	UPROPERTY(
		EditDefaultsOnly,
		Category = "UI",
		meta = (
			RowType = "/Script/CommonUI.CommonInputActionDataBase",
			DisplayName = "Open Input Action"))
	FDataTableRowHandle m_OpenInputAction;
};

/**
 * 특정 GameMode의 HUD와 각 UI 도메인에서 사용할 위젯 클래스를 정의합니다.
 * Soft Class를 사용하여 Config 로드 시 모든 위젯 클래스를 함께 로드하지 않습니다.
 * 런타임 조회와 Layer 선택은 UPFUIManagerSubsystem이 담당합니다.
 */
UCLASS()
class PROJECT_SIH_API UPFUIConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 현재 GameMode에서 기본 HUD로 사용할 위젯 클래스를 반환합니다. */
	const TSoftClassPtr<UPFGameUIRootBase>& GetHUDLayOutClass() const;

	/** UI.Game ID에 대응하는 위젯 정의를 찾습니다. */
	const FPFUIWidgetDefinition* FindGameWidgetDefinition(const FGameplayTag& WidgetID) const;

	/** UI.Screen ID에 대응하는 위젯 정의를 찾습니다. */
	const FPFUIWidgetDefinition* FindScreenWidgetDefinition(const FGameplayTag& WidgetID) const;

	/** UI.Modal ID에 대응하는 위젯 정의를 찾습니다. */
	const FPFUIWidgetDefinition* FindModalWidgetDefinition(const FGameplayTag& WidgetID) const;

	/** 세 UI 도메인의 위젯 정의를 ID 기준 단일 Map으로 복사합니다. */
	void GetWidgetDefinitions(TMap<FGameplayTag, FPFUIWidgetDefinition>& OutDefinitions) const;

private:
	/** GameStack의 최하단에 배치되어 게임 플레이 중 기본 HUD로 사용되는 위젯 클래스입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "UI|HUD", meta = (DisplayName = "HUD Layout Class"))
	TSoftClassPtr<UPFGameUIRootBase> m_HUDLayOutClass;

	/** UI.Game 태그와 GameStack에 표시할 위젯 정의를 매핑합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "UI|Game", meta = (Categories = "UI.Game", DisplayName = "Game Widgets"))
	TMap<FGameplayTag, FPFUIWidgetDefinition> m_GameWidgetDefinitions;

	/** UI.Screen 태그와 ScreenStack에 표시할 위젯 정의를 매핑합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "UI|Screen", meta = (Categories = "UI.Screen", DisplayName = "Screen Widgets"))
	TMap<FGameplayTag, FPFUIWidgetDefinition> m_ScreenWidgetDefinitions;

	/** UI.Modal 태그와 ModalStack에 표시할 위젯 정의를 매핑합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "UI|Modal", meta = (Categories = "UI.Modal", DisplayName = "Modal Widgets"))
	TMap<FGameplayTag, FPFUIWidgetDefinition> m_ModalWidgetDefinitions;
};
