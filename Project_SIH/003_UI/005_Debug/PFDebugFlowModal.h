#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/003_UI/000_Foundation/PFActivatableWidget.h"
#include "PFDebugFlowModal.generated.h"

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFDebugFlowModal : public UPFActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Debug")
	bool TrySkipInvestigation();
};
