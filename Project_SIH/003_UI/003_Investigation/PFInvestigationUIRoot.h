#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/003_UI/000_Foundation/PFGameUIRootBase.h"
#include "PFInvestigationUIRoot.generated.h"

/**
 * Investigation GameMode에서 유지되는 도메인 UI Root입니다.
 * 공통 Config 입력 라우팅에 더해 Investigation의 사건 시작 요청만 제공합니다.
 */
UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFInvestigationUIRoot : public UPFGameUIRootBase
{
	GENERATED_BODY()

public:
	/** 사건 시작 Screen 또는 임시 HUD 버튼에서 전달한 Case의 시작을 요청합니다. */
	UFUNCTION(BlueprintCallable, Category = "UI|Investigation")
	bool TryStartCase(FGameplayTag CaseID);
};
