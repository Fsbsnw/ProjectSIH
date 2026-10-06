#include "PFBattleActionPanelWidget.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"

void UPFBattleActionPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Attack->OnClicked().AddUObject(
		this, &ThisClass::HandleBasicAttackClicked);

	Button_Cancel->OnClicked().AddUObject(
		this, &ThisClass::HandleCancelClicked);

	SetControls(false, false);
	SetHintText(FText::GetEmpty());
}

void UPFBattleActionPanelWidget::SetControls(
	const bool bCanSelectAction,
	const bool bSelectingTarget)
{
	Button_Attack->SetIsEnabled(bCanSelectAction);

	const bool bCanCancel =
		bCanSelectAction && bSelectingTarget;

	Button_Cancel->SetIsEnabled(bCanCancel);
	Button_Cancel->SetVisibility(
		bCanCancel
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);

	Text_Attack->SetText(
		bSelectingTarget
			? NSLOCTEXT(
				"PFBattleActionPanel", "ConfirmAttack", "공격 확정")
			: NSLOCTEXT(
				"PFBattleActionPanel", "BasicAttack", "기본공격"));
}

void UPFBattleActionPanelWidget::SetHintText(
	const FText& HintText)
{
	Text_Hint->SetText(HintText);
}

UPFBattleActionPanelWidget::FBasicAttackClickedEvent&
UPFBattleActionPanelWidget::OnBasicAttackClicked()
{
	return m_OnBasicAttackClicked;
}

UPFBattleActionPanelWidget::FCancelClickedEvent&
UPFBattleActionPanelWidget::OnCancelClicked()
{
	return m_OnCancelClicked;
}

void UPFBattleActionPanelWidget::HandleBasicAttackClicked()
{
	m_OnBasicAttackClicked.Broadcast();
}

void UPFBattleActionPanelWidget::HandleCancelClicked()
{
	m_OnCancelClicked.Broadcast();
}
