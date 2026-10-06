#include "PFEnemy.h"

#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleMessages.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFBattleActionCommandReceiver.h"

EPFBattleSide APFEnemy::GetBattleSide() const
{
	return EPFBattleSide::Enemy;
}

void APFEnemy::BeginPlay()
{
	Super::BeginPlay();

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

void APFEnemy::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(m_ActionSelectionTimer);
	m_ActionStateHandle.Unregister();

	Super::EndPlay(EndPlayReason);
}

void APFEnemy::HandleActionState(
	FGameplayTag Channel,
	const FPFBattleActionStateMessage& Message)
{
	if (Channel ==
		SIHGameplayTags::Message_Battle_ActionState_ActionSelection.GetTag())
	{
		GetWorldTimerManager().ClearTimer(m_ActionSelectionTimer);

		if (Message.m_ActionOwner.Get() != this)
		{
			return;
		}

		GetWorldTimerManager().SetTimer(
			m_ActionSelectionTimer,
			this,
			&ThisClass::SelectAction,
			1.0f,
			false);
	}
	else if (Channel ==
		SIHGameplayTags::Message_Battle_ActionState_TargetSelection.GetTag())
	{
		GetWorldTimerManager().ClearTimer(m_ActionSelectionTimer);
		if (Message.m_ActionOwner.Get() != this)
		{
			return;
		}

		GetWorldTimerManager().SetTimer(
			m_ActionSelectionTimer,
			this,
			&ThisClass::ConfirmAction,
			1.0f,
			false);
	}
	else if (Channel ==
		SIHGameplayTags::Message_Battle_ActionState_Processing.GetTag())
	{
		GetWorldTimerManager().ClearTimer(m_ActionSelectionTimer);
	}
	else if (Channel ==
		SIHGameplayTags::Message_Battle_ActionState_Cleared.GetTag())
	{
		GetWorldTimerManager().ClearTimer(m_ActionSelectionTimer);
	}
}

void APFEnemy::SelectAction()
{
	IPFBattleActionCommandReceiver* Receiver =
		Cast<IPFBattleActionCommandReceiver>(
			GetWorld()->GetAuthGameMode());

	if (!Receiver)
	{
		PF_LOG(TEXT("Enemy battle action command receiver is unavailable"));
		return;
	}

	const FGameplayTag ActionTag =
		SIHGameplayTags::Action_Ability_BasicAttack.GetTag();

	Receiver->SelectBattleAction(this, ActionTag);
}

void APFEnemy::ConfirmAction()
{
	IPFBattleActionCommandReceiver* Receiver =
		Cast<IPFBattleActionCommandReceiver>(GetWorld()->GetAuthGameMode());
	if (!Receiver)
	{
		PF_LOG(TEXT("Enemy battle action command receiver is unavailable"));
		return;
	}

	Receiver->ConfirmBattleAction(this);
}
