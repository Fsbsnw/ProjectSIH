#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayTagContainer.h"
#include "PFBattleWeaknessSlotsWidget.generated.h"

class UCommonTextBlock;
struct FPFBattleWeaknessSlotsMessage;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFBattleWeaknessSlotsWidget
	: public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void HandleWeaknessSlotsInitialized(
		FGameplayTag Channel,
		const FPFBattleWeaknessSlotsMessage& Message);

	void SetSlots(
		const TArray<FGameplayTag>& VisibleElementIDs);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Weakness1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Weakness2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Weakness3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Weakness4;

	FGameplayMessageListenerHandle m_WeaknessMessageHandle;
};
