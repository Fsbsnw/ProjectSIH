#include "PFClueSubmissionEntryWidget.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"

void UPFClueSubmissionEntryWidget::SetClueID(const FGameplayTag& ClueID)
{
	m_ClueID = ClueID;
	Text_Clue->SetText(FText::FromString(ClueID.ToString()));
}

UPFClueSubmissionEntryWidget::FClueSelectedEvent& UPFClueSubmissionEntryWidget::OnClueClicked()
{
	return m_OnClueSelected;
}

void UPFClueSubmissionEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Button_ClueSubmission->OnClicked().AddUObject(this, &UPFClueSubmissionEntryWidget::HandleSubmissionButtonClicked);
}

void UPFClueSubmissionEntryWidget::HandleSubmissionButtonClicked()
{
	m_OnClueSelected.Broadcast(m_ClueID);
}
