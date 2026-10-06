#include "PFBattleTargetEntryWidget.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/003_Combat/001_Characters/PFBattleCharacterBase.h"

void UPFBattleTargetEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Target->OnClicked().AddUObject(
		this, &ThisClass::HandleClicked);

	Button_Target->SetIsEnabled(false);
	SetSelected(false);
}

void UPFBattleTargetEntryWidget::SetTarget(
	APFBattleCharacterBase* Target,
	const bool bCanSelect)
{
	m_Target = Target;

	if (!IsValid(Target))
	{
		PF_LOG(TEXT("Cannot assign target entry: target actor is unavailable"));

		Button_Target->SetIsEnabled(false);
		SetSelected(false);
		return;
	}

	Button_Target->SetIsEnabled(bCanSelect);
}

void UPFBattleTargetEntryWidget::SetSelected(
	const bool bSelected)
{
	const ESlateVisibility SelectionVisibility =
		bSelected
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Hidden;

	Border_Selected->SetVisibility(SelectionVisibility);
	Text_SelectedMarker->SetVisibility(SelectionVisibility);
}

APFBattleCharacterBase*
UPFBattleTargetEntryWidget::GetTarget() const
{
	return m_Target.Get();
}

UPFBattleTargetEntryWidget::FTargetClickedEvent&
UPFBattleTargetEntryWidget::OnTargetClicked()
{
	return m_OnTargetClicked;
}

void UPFBattleTargetEntryWidget::HandleClicked()
{
	APFBattleCharacterBase* Target = m_Target.Get();

	if (!IsValid(Target))
	{
		PF_LOG(TEXT("Cannot select target entry: target actor is unavailable"));
		return;
	}

	m_OnTargetClicked.Broadcast(Target);
}
