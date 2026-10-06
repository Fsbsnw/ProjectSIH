#include "PFBattleActionSelectionWidget.h"

#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "PFBattleActionPanelWidget.h"
#include "PFBattleTargetIndicatorWidget.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleMessages.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/003_Combat/001_Characters/PFBattleCharacterBase.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFBattleActionCommandReceiver.h"

void UPFBattleActionSelectionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ActionPanel->OnBasicAttackClicked().AddUObject(
		this, &ThisClass::HandleBasicAttackClicked);

	ActionPanel->OnCancelClicked().AddUObject(
		this, &ThisClass::HandleCancelClicked);

	TargetIndicator->OnTargetClicked().AddUObject(
		this, &ThisClass::HandleTargetClicked);
}

void UPFBattleActionSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	m_ActionOwner.Reset();
	m_bCanPlayerSelectAction = false;
	ClearSelection();
	RefreshControls();

	FGameplayMessageListenerParams<FPFBattleActionStateMessage> Params;
	Params.MatchType = EGameplayMessageMatch::PartialMatch;
	Params.SetMessageReceivedCallback(
		this, &ThisClass::HandleActionState);

	m_ActionStateHandle =
		UGameplayMessageSubsystem::Get(this)
			.RegisterListener<FPFBattleActionStateMessage>(
				SIHGameplayTags::Message_Battle_ActionState.GetTag(),
				Params);
}

void UPFBattleActionSelectionWidget::NativeDestruct()
{
	m_ActionStateHandle.Unregister();

	m_ActionOwner.Reset();
	m_bCanPlayerSelectAction = false;
	ClearSelection();
	RefreshControls();

	Super::NativeDestruct();
}

void UPFBattleActionSelectionWidget::HandleActionState(
	FGameplayTag Channel,
	const FPFBattleActionStateMessage& Message)
{
	m_ActionOwner = Message.m_ActionOwner.Get();
	m_bSelectingTarget =
		Channel == SIHGameplayTags::Message_Battle_ActionState_TargetSelection.GetTag();
	const bool bSelectionPhase =
		Channel == SIHGameplayTags::Message_Battle_ActionState_ActionSelection.GetTag()
		|| m_bSelectingTarget;
	m_bCanPlayerSelectAction = bSelectionPhase && Message.m_bCanPlayerSelectAction;
	m_TargetCount = Message.m_TargetSelection.m_TargetCount;

	if (m_bSelectingTarget)
	{
		TargetIndicator->ShowTargets(
			Message.m_TargetSelection, m_bCanPlayerSelectAction);
	}
	else
	{
		TargetIndicator->ClearTargets();
	}

	RefreshControls();
}

void UPFBattleActionSelectionWidget::HandleBasicAttackClicked()
{
	if (!m_bCanPlayerSelectAction || !m_ActionOwner.IsValid())
	{
		PF_LOG(TEXT("Cannot select an attack: action selection is unavailable"));
		return;
	}

	IPFBattleActionCommandReceiver* Receiver = GetCommandReceiver();
	if (!Receiver)
	{
		return;
	}

	if (!m_bSelectingTarget)
	{
		const FGameplayTag ActionTag =
			SIHGameplayTags::Action_Ability_BasicAttack.GetTag();

		if (!Receiver->SelectBattleAction(m_ActionOwner.Get(), ActionTag))
		{
			ActionPanel->SetHintText(NSLOCTEXT(
				"PFBattleActionSelection", "TargetQueryFailed",
				"공격할 대상을 찾을 수 없습니다"));
			return;
		}

		return;
	}

	if (!Receiver->ConfirmBattleAction(m_ActionOwner.Get()))
	{
		ActionPanel->SetHintText(NSLOCTEXT(
			"PFBattleActionSelection", "ActionRejected",
			"공격을 실행할 수 없습니다"));
	}
}

void UPFBattleActionSelectionWidget::HandleTargetClicked(
	APFBattleCharacterBase* Target)
{
	if (!m_bSelectingTarget
		|| !m_bCanPlayerSelectAction
		|| m_TargetCount != EPFTargetCount::Single)
	{
		return;
	}

	IPFBattleActionCommandReceiver* Receiver = GetCommandReceiver();
	if (!Receiver)
	{
		return;
	}

	Receiver->SelectBattleTarget(m_ActionOwner.Get(), Target);
}

void UPFBattleActionSelectionWidget::HandleCancelClicked()
{
	if (!m_bSelectingTarget || !m_bCanPlayerSelectAction)
	{
		return;
	}

	IPFBattleActionCommandReceiver* Receiver = GetCommandReceiver();
	if (!Receiver)
	{
		return;
	}

	Receiver->CancelBattleActionSelection(m_ActionOwner.Get());
}

void UPFBattleActionSelectionWidget::ClearSelection()
{
	m_bSelectingTarget = false;
	m_TargetCount = EPFTargetCount::Single;

	TargetIndicator->ClearTargets();
}

IPFBattleActionCommandReceiver*
UPFBattleActionSelectionWidget::GetCommandReceiver() const
{
	IPFBattleActionCommandReceiver* Receiver =
		Cast<IPFBattleActionCommandReceiver>(GetWorld()->GetAuthGameMode());
	if (!Receiver)
	{
		PF_LOG(TEXT("Battle action command receiver is unavailable"));
	}
	return Receiver;
}

void UPFBattleActionSelectionWidget::RefreshControls()
{
	const bool bCanSelect =
		m_bCanPlayerSelectAction && m_ActionOwner.IsValid();

	ActionPanel->SetControls(bCanSelect, m_bSelectingTarget);

	if (!bCanSelect)
	{
		ActionPanel->SetHintText(NSLOCTEXT(
			"PFBattleActionSelection", "Waiting",
			"행동 대기 중"));
	}
	else if (!m_bSelectingTarget)
	{
		ActionPanel->SetHintText(NSLOCTEXT(
			"PFBattleActionSelection", "ChooseAction",
			"행동을 선택하세요"));
	}
	else if (m_TargetCount == EPFTargetCount::All)
	{
		ActionPanel->SetHintText(NSLOCTEXT(
			"PFBattleActionSelection", "AllTargets",
			"전체 대상에게 공격합니다"));
	}
	else
	{
		ActionPanel->SetHintText(NSLOCTEXT(
			"PFBattleActionSelection", "ChooseTarget",
			"대상을 선택하세요"));
	}
}
