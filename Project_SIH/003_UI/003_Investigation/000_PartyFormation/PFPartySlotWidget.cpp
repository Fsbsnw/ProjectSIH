#include "PFPartySlotWidget.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"

void UPFPartySlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	m_DefaultCharacterBrush = Image_Character->GetBrush();

	Button_Slot->OnClicked().AddUObject(
		this,
		&UPFPartySlotWidget::HandleClicked);
}

void UPFPartySlotWidget::SetSlot(
	int32 SlotIndex,
	FGameplayTag CharacterID,
	const FText& DisplayName,
	UTexture2D* Icon)
{
	m_SlotIndex = SlotIndex;

	Text_SlotNumber->SetText(FText::FromString(
		FString::Printf(TEXT("%02d"), SlotIndex + 1)));

	if (!CharacterID.IsValid())
	{
		Text_CharacterName->SetText(
			NSLOCTEXT("PFPartySlotWidget", "EmptySlot", "선택 대기"));
		Image_Character->SetBrush(m_DefaultCharacterBrush);
		Image_Character->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	Text_CharacterName->SetText(
		DisplayName.IsEmpty()
			? FText::FromString(CharacterID.ToString())
			: DisplayName);

	if (IsValid(Icon))
	{
		Image_Character->SetBrushFromTexture(Icon);
	}
	else
	{
		Image_Character->SetBrush(m_DefaultCharacterBrush);
	}
	Image_Character->SetVisibility(
		ESlateVisibility::SelfHitTestInvisible);
}

UPFPartySlotWidget::FSlotClickedEvent& UPFPartySlotWidget::OnSlotClicked()
{
	return m_OnSlotClicked;
}

void UPFPartySlotWidget::HandleClicked()
{
	if (m_SlotIndex != INDEX_NONE)
	{
		m_OnSlotClicked.Broadcast(m_SlotIndex);
	}
}
