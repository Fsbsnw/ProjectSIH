#include "PFBattleTargetIndicatorWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/PlayerController.h"
#include "PFBattleTargetEntryWidget.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/003_Combat/001_Characters/PFBattleCharacterBase.h"

void UPFBattleTargetIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ClearTargets();
}

void UPFBattleTargetIndicatorWidget::NativeDestruct()
{
	ClearTargets();
	Super::NativeDestruct();
}

void UPFBattleTargetIndicatorWidget::ShowTargets(
	const FPFBattleTargetSelection& Selection,
	bool bAllowTargetSelection)
{
	ClearTargets();

	if (!m_TargetEntryClass)
	{
		PF_LOG(TEXT("Target Entry Class is not configured"));
		return;
	}

	APlayerController* PlayerController = GetOwningPlayer();
	if (!IsValid(PlayerController))
	{
		PF_LOG(TEXT("Cannot show targets: owning player is unavailable"));
		return;
	}

	const bool bAllTargets =
		Selection.m_TargetCount == EPFTargetCount::All;

	for (APFBattleCharacterBase* Target : Selection.m_ValidTargets)
	{
		if (!IsValid(Target))
		{
			PF_LOG(TEXT("Cannot create target entry: candidate actor is unavailable"));
			continue;
		}

		UPFBattleTargetEntryWidget* Entry =
			CreateWidget<UPFBattleTargetEntryWidget>(
				PlayerController, m_TargetEntryClass);

		if (!IsValid(Entry))
		{
			PF_LOG(TEXT("Failed to create target entry"));
			ClearTargets();
			return;
		}

		Entry->SetTarget(Target, bAllowTargetSelection && !bAllTargets);
		Entry->SetSelected(
			bAllTargets || Target == Selection.m_DefaultTarget);

		Entry->OnTargetClicked().AddUObject(
			this, &ThisClass::HandleTargetClicked);

		UCanvasPanelSlot* EntrySlot =
			Canvas_Targets->AddChildToCanvas(Entry);

		EntrySlot->SetAnchors(FAnchors(0.0f, 0.0f));
		EntrySlot->SetAlignment(FVector2D(0.5, 0.5));
		EntrySlot->SetAutoSize(true);

		Entry->SetVisibility(ESlateVisibility::Hidden);
		m_Entries.Add(Entry);
	}

	if (!m_Entries.IsEmpty())
	{
		RefreshTargetPositions();
		if (!m_Entries.IsEmpty())
		{
			SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
}

void UPFBattleTargetIndicatorWidget::RefreshTargetPositions()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!IsValid(PlayerController))
	{
		PF_LOG(TEXT("Cannot update target positions: owning player is unavailable"));
		ClearTargets();
		return;
	}

	for (int32 Index = m_Entries.Num() - 1; Index >= 0; --Index)
	{
		UPFBattleTargetEntryWidget* Entry = m_Entries[Index];
		APFBattleCharacterBase* Target = Entry->GetTarget();

		if (!IsValid(Target))
		{
			Entry->OnTargetClicked().RemoveAll(this);
			Entry->RemoveFromParent();
			m_Entries.RemoveAtSwap(Index);
			continue;
		}

		FVector2D WidgetPosition;
		const bool bProjected =
			UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
				PlayerController,
				Target->GetActorLocation() + m_TargetWorldOffset,
				WidgetPosition,
				false);

		if (!bProjected)
		{
			Entry->SetVisibility(ESlateVisibility::Hidden);
			continue;
		}

		UWidgetLayoutLibrary::SlotAsCanvasSlot(Entry)
			->SetPosition(WidgetPosition);

		Entry->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (m_Entries.IsEmpty())
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPFBattleTargetIndicatorWidget::SetSelectedTarget(
	APFBattleCharacterBase* Target)
{
	for (UPFBattleTargetEntryWidget* Entry : m_Entries)
	{
		Entry->SetSelected(
			Target != nullptr && Entry->GetTarget() == Target);
	}
}

void UPFBattleTargetIndicatorWidget::HandleTargetClicked(
	APFBattleCharacterBase* Target)
{
	m_OnTargetClicked.Broadcast(Target);
}

void UPFBattleTargetIndicatorWidget::ClearTargets()
{
	SetVisibility(ESlateVisibility::Collapsed);

	for (UPFBattleTargetEntryWidget* Entry : m_Entries)
	{
		Entry->OnTargetClicked().RemoveAll(this);
	}

	Canvas_Targets->ClearChildren();
	m_Entries.Reset();
}

UPFBattleTargetIndicatorWidget::FTargetClickedEvent&
UPFBattleTargetIndicatorWidget::OnTargetClicked()
{
	return m_OnTargetClicked;
}
