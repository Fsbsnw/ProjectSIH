#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Styling/SlateBrush.h"
#include "PFPartySlotWidget.generated.h"

class UCommonButtonBase;
class UCommonTextBlock;
class UImage;
class UTexture2D;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFPartySlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FSlotClickedEvent, int32);

	void SetSlot(
		int32 SlotIndex,
		FGameplayTag CharacterID,
		const FText& DisplayName,
		UTexture2D* Icon);

	FSlotClickedEvent& OnSlotClicked();

protected:
	virtual void NativeOnInitialized() override;

private:
	void HandleClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Slot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_SlotNumber;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_CharacterName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Character;

	int32 m_SlotIndex = INDEX_NONE;

	UPROPERTY(Transient)
	FSlateBrush m_DefaultCharacterBrush;

	FSlotClickedEvent m_OnSlotClicked;
};
