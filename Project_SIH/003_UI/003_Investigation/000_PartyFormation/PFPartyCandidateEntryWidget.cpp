#include "PFPartyCandidateEntryWidget.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"

void UPFPartyCandidateEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	m_DefaultIconBrush = Image_CharacterIcon->GetBrush();

	Button_Candidate->OnClicked().AddUObject(
		this,
		&UPFPartyCandidateEntryWidget::HandleClicked);
}

void UPFPartyCandidateEntryWidget::SetCandidate(
	FGameplayTag CharacterID,
	const FText& DisplayName,
	int32 Level,
	UTexture2D* Icon)
{
	m_CharacterID = CharacterID;

	Text_CharacterName->SetText(
		DisplayName.IsEmpty()
			? FText::FromString(CharacterID.ToString())
			: DisplayName);

	Text_Level->SetText(FText::FromString(
		FString::Printf(TEXT("LV. %d"), Level)));

	if (IsValid(Icon))
	{
		Image_CharacterIcon->SetBrushFromTexture(Icon);
	}
	else
	{
		Image_CharacterIcon->SetBrush(m_DefaultIconBrush);
	}

	SetSelected(false);
}

void UPFPartyCandidateEntryWidget::SetSelected(bool bSelected)
{
	Border_Selected->SetVisibility(
		bSelected
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
}

UPFPartyCandidateEntryWidget::FCandidateClickedEvent&
UPFPartyCandidateEntryWidget::OnCandidateClicked()
{
	return m_OnCandidateClicked;
}

void UPFPartyCandidateEntryWidget::HandleClicked()
{
	if (m_CharacterID.IsValid())
	{
		m_OnCandidateClicked.Broadcast(m_CharacterID);
	}
}
