#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PFUIManagerSubsystem.generated.h"

class APlayerController;
class UCommonActivatableWidget;
class ULocalPlayer;
class UWorld;
class UPFActivatableWidget;
class UPFPrimaryGameLayout;
class UPFUIConfig;

/** UI Root가 CommonUI Action을 Widget ID 표시 요청으로 연결할 런타임 바인딩입니다. */
struct FPFUIInputBinding
{
	FGameplayTag m_WidgetID;
	FDataTableRowHandle m_InputAction;
};

/**
 * LocalPlayer별 UI 진입점입니다.
 * PrimaryGameLayout의 생성과 Viewport 연결, GameMode별 Config 선택, 태그 기반 위젯 라우팅을 담당합니다.
 * 외부 시스템은 실제 위젯 클래스나 Stack을 참조하지 않고 ShowWidget에 의미 ID만 전달합니다.
 */
UCLASS()
class PROJECT_SIH_API UPFUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	/** 현재 LocalPlayer에 이미 연결된 Controller가 있으면 초기 UI 구성을 준비합니다. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 생성한 Layout과 런타임 참조를 정리합니다. */
	virtual void Deinitialize() override;

	/** LocalPlayer에 연결된 Controller가 변경되면 Layout과 GameMode별 UI를 갱신합니다. */
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

	/** 현재 로컬 플레이어의 최상위 Layout을 반환합니다. */
	UPFPrimaryGameLayout* GetPrimaryGameLayout() const;

	/** 현재 GameMode에 선택된 UI Config를 반환합니다. 공용 Config는 포함하지 않습니다. */
	const UPFUIConfig* GetUIConfig() const;

	/** GameMode 우선 규칙을 적용한 현재 Root의 UI 입력 바인딩을 반환합니다. */
	void GetResolvedInputBindings(TArray<FPFUIInputBinding>& OutBindings) const;

	/** UI.Game, UI.Screen, UI.Modal 도메인을 판별하여 대응하는 Layer에 위젯을 Push합니다. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	UCommonActivatableWidget* ShowWidget(FGameplayTag WidgetID);

private:
	/** GameStack을 비우고 현재 Config의 HUD를 기반 위젯으로 Push합니다. */
	UCommonActivatableWidget* ShowHUDLayout();

	/** LocalPlayer의 Controller가 준비되거나 교체될 때 Layout과 HUD를 갱신합니다. */
	void HandlePlayerControllerChanged(APlayerController* PlayerController);

	/** 이전 World에 연결된 Layout과 HUD 참조를 정리합니다. */
	void ReleasePrimaryGameLayout();

	/** 공용 Config와 Controller의 현재 GameMode에 대응하는 Config를 선택합니다. */
	bool LoadUIConfigsForGameMode(const APlayerController* PlayerController);

	/** UI.Game ID에 대응하는 위젯을 Game Layer에 Push합니다. */
	UCommonActivatableWidget* ShowGameWidget(const FGameplayTag& WidgetID);

	/** UI.Screen ID에 대응하는 위젯을 Screen Layer에 Push합니다. */
	UCommonActivatableWidget* ShowScreenWidget(const FGameplayTag& WidgetID);

	/** UI.Modal ID에 대응하는 위젯을 Modal Layer에 Push합니다. */
	UCommonActivatableWidget* ShowModalWidget(const FGameplayTag& WidgetID);

	/** Config의 Widget Definition에서 Soft Class를 동기 로드합니다. */
	TSubclassOf<UCommonActivatableWidget> LoadWidgetClass(const struct FPFUIWidgetDefinition* WidgetDefinition, const FGameplayTag& WidgetID) const;

	/** 모든 GameMode에서 공통으로 조회하는 Config입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UPFUIConfig> m_GlobalUIConfig;

	/** 현재 GameMode에 선택된 Config입니다. 동일한 Widget ID는 이 Config를 우선합니다. */
	UPROPERTY(Transient)
	TObjectPtr<UPFUIConfig> m_GameModeUIConfig;

	/** 로컬 플레이어 화면에 연결된 최상위 Layout입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UPFPrimaryGameLayout> m_PrimaryGameLayout;

	/** 현재 Layout을 생성한 World입니다. World 전환 시 이전 UI를 재사용하지 않습니다. */
	TWeakObjectPtr<UWorld> m_LayoutWorld;

};
