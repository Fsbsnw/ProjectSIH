#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/003_UI/000_Foundation/PFActivatableWidget.h"
#include "PFBattleResultScreen.generated.h"

class UCommonButtonBase;
class UCommonTextBlock;
struct FPFBattleResult;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFBattleResultScreen : public UPFActivatableWidget
{
	GENERATED_BODY()

public:
	void SetResult(const FPFBattleResult& Result);

protected:
	virtual void NativeOnInitialized() override;

private:
	void HandleExitClicked();

	UPROPERTY(meta = (BindWidget, DisplayName = "Case ID Text"))
	TObjectPtr<UCommonTextBlock> Text_CaseID;

	UPROPERTY(meta = (BindWidget, DisplayName = "Result Text"))
	TObjectPtr<UCommonTextBlock> Text_Result;

	UPROPERTY(meta = (BindWidget, DisplayName = "Exit Button"))
	TObjectPtr<UCommonButtonBase> Button_Exit;
};
