#include "PFDialogueChoiceEntryWidget.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"

void UPFDialogueChoiceEntryWidget::SetChoice(
	const FPFDialogueChoice& Choice)
{
	m_ChoiceID = Choice.m_ChoiceID;
	Text_Choice->SetText(Choice.m_Text);
}

UPFDialogueChoiceEntryWidget::FChoiceSelectedEvent&
UPFDialogueChoiceEntryWidget::OnChoiceClicked()
{
	return m_OnChoiceSelected;
}

void UPFDialogueChoiceEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Button_Choice->OnClicked().AddUObject(
		this,
		&UPFDialogueChoiceEntryWidget::HandleChoiceButtonClicked);
}

void UPFDialogueChoiceEntryWidget::HandleChoiceButtonClicked()
{
	m_OnChoiceSelected.Broadcast(m_ChoiceID);
}
