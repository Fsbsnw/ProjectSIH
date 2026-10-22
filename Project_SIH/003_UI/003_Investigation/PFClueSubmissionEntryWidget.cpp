#include "PFClueSubmissionEntryWidget.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"

void UPFClueSubmissionEntryWidget::SetEntryID(const FGameplayTag& EntryID)
{
	m_EntryID = EntryID;
	Text_Clue->SetText(FText::FromString(EntryID.ToString()));
}

UPFClueSubmissionEntryWidget::FEntrySelectedEvent& UPFClueSubmissionEntryWidget::OnEntryClicked()
{
	return m_OnEntrySelected;
}

void UPFClueSubmissionEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Button_ClueSubmission->OnClicked().AddUObject(this, &UPFClueSubmissionEntryWidget::HandleSubmissionButtonClicked);
}

void UPFClueSubmissionEntryWidget::HandleSubmissionButtonClicked()
{
	m_OnEntrySelected.Broadcast(m_EntryID);
}
