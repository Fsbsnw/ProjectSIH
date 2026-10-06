#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/003_UI/000_Foundation/PFActivatableWidget.h"
#include "PFGameUIRootBase.generated.h"

/**
 * GameMode HUD가 공통으로 상속하는 UI Root입니다.
 * 현재 Global/GameMode UI Config의 표시 입력을 CommonUI에 등록하고 Widget ID 요청으로 변환합니다.
 */
UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFGameUIRootBase : public UPFActivatableWidget
{
	GENERATED_BODY()

protected:
	/** 현재 UI Config에 설정된 단순 Widget 표시 입력을 등록합니다. */
	virtual void NativeOnInitialized() override;

private:
	/** 하나의 CommonUI Action을 Widget 표시 요청으로 등록합니다. */
	void RegisterWidgetInput(const FDataTableRowHandle& InputAction, FGameplayTag WidgetID);

	/** 등록된 입력을 UIManager의 태그 기반 표시 요청으로 전달합니다. */
	void HandleWidgetInput(FGameplayTag WidgetID);
};
