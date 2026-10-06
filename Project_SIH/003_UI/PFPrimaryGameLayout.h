#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "PFPrimaryGameLayout.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;
class UPFUIManagerSubsystem;

/**
 * 로컬 플레이어 UI의 최상위 Layout입니다.
 * 위젯 종류나 게임 규칙은 알지 않고 등록된 Layer Stack과 활성 순서만 관리합니다.
 * 위젯 Push와 Pop은 UPFUIManagerSubsystem을 통해서만 수행합니다.
 */
UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFPrimaryGameLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** 지정한 Layer에서 현재 활성화된 최상위 위젯을 반환합니다. */
	UCommonActivatableWidget* GetActiveWidgetInLayer(FGameplayTag LayerID) const;

	/** Game Layer에서 현재 활성화된 위젯을 반환합니다. */
	UCommonActivatableWidget* GetActiveGameWidget() const;

	/** Screen Layer에서 현재 활성화된 위젯을 반환합니다. */
	UCommonActivatableWidget* GetActiveScreenWidget() const;

	/** Modal Layer에서 현재 활성화된 위젯을 반환합니다. */
	UCommonActivatableWidget* GetActiveModalWidget() const;

	/** 현재 활성화된 Modal 위젯이 있는지 확인합니다. */
	bool IsModalActive() const;

protected:
	/** BindWidget으로 연결된 기본 Stack을 Layer Map에 등록합니다. */
	virtual void NativeOnInitialized() override;

	/** C++ 파생 Layout에서 추가 Stack을 사용할 때 Layer ID와 함께 등록합니다. */
	void RegisterLayer(FGameplayTag LayerID, UCommonActivatableWidgetStack* LayerStack);

	/** HUD와 게임 플레이 중 오버레이를 쌓는 기본 Layer입니다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> m_GameStack;

	/** 인벤토리와 설정 등 전체 화면 UI를 쌓는 Layer입니다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> m_ScreenStack;

	/** 확인창과 경고창처럼 다른 UI보다 우선하는 UI를 쌓는 Layer입니다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> m_ModalStack;

private:
	friend class UPFUIManagerSubsystem;

	/** 지정한 Layer에 위젯을 생성하여 Push합니다. */
	UCommonActivatableWidget* PushWidgetToLayer(FGameplayTag LayerID, TSubclassOf<UCommonActivatableWidget> WidgetClass);

	/** 지정한 Layer에 쌓인 모든 위젯을 제거합니다. */
	void ClearLayer(FGameplayTag LayerID);

	/** GameMode별 UI Config 교체 시 모든 Stack을 초기화합니다. */
	void ClearAllWidgets();

	/** Layer ID에 등록된 Stack을 찾습니다. */
	UCommonActivatableWidgetStack* FindLayer(FGameplayTag LayerID) const;

	/** GameplayTag Layer ID와 실제 Stack의 런타임 매핑입니다. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetStack>> m_Layers;
};
