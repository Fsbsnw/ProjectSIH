#include "PFBattleRuntime.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleMessages.h"
#include "Project_SIH/002_Systems/003_Combat/001_Characters/PFBattleCharacterBase.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFBattleActionParticipantInterface.h"
#include "Project_SIH/002_Systems/003_Combat/PFTurnSystem.h"
#include "Project_SIH/002_Systems/009_AbilitySystem/PFAbilitySystemComponent.h"

EPFPhaseStartResult UPFBattleRuntime::StartBattle(
	const FPFBattleRuntimeContext& Context)
{
	if (m_CaseID.IsValid())
	{
		PF_LOG(TEXT("Battle runtime is already active"));
		return EPFPhaseStartResult::AlreadyActive;
	}

	if (!Context.IsValid())
	{
		PF_LOG(TEXT("Battle runtime context is invalid"));
		return EPFPhaseStartResult::InvalidContext;
	}

	int32 AllyCount = 0;
	int32 EnemyCount = 0;

	for (APFBattleCharacterBase* Participant
		: Context.m_Participants)
	{
		if (!IsValid(Participant))
		{
			PF_LOG(TEXT("Battle participant is invalid"));
			return EPFPhaseStartResult::InvalidContext;
		}

		if (!Cast<IPFBattleActionParticipantInterface>(
			Participant))
		{
			PF_LOG(
				TEXT(
					"Battle participant does not implement the "
					"action participant interface. Participant=%s"),
				*GetNameSafe(Participant));
			return EPFPhaseStartResult::InvalidContext;
		}

		if (Participant->GetBattleSide()
			== EPFBattleSide::Ally)
		{
			++AllyCount;
		}

		if (Participant->GetBattleSide()
			== EPFBattleSide::Enemy)
		{
			++EnemyCount;
		}
	}

	if (AllyCount != FPFPartyData::RequiredMemberCount
		|| EnemyCount != 1)
	{
		PF_LOG(
			TEXT(
				"Battle participant composition is invalid. "
				"Allies=%d, Enemies=%d"),
			AllyCount,
			EnemyCount);
		return EPFPhaseStartResult::InvalidContext;
	}

	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		PF_LOG(TEXT("GameplayMessageSubsystem is unavailable"));
		return EPFPhaseStartResult::NotReady;
	}

	if (!IsValid(m_TurnSystem))
	{
		PF_LOG(TEXT("TurnSystem is unavailable"));
		return EPFPhaseStartResult::NotReady;
	}

	if (!m_TurnSystem->StartManagingTurns(
		Context.m_Participants))
	{
		PF_LOG(TEXT("Failed to start managing turns"));
		return EPFPhaseStartResult::InvalidContext;
	}

	m_ResultOnVictory = Context.m_InitialWeaknessCounts
		.DetermineResultOnVictory();
	m_CaseID = Context.m_CaseID;
	m_Participants = Context.m_Participants;
	BindActionCompletionDelegates();

	RequestActionSelection();

	return EPFPhaseStartResult::Started;
}

bool UPFBattleRuntime::GetValidTargetsAndDefaultTarget(
	APFBattleCharacterBase* Requester,
	const FGameplayTag ActionTag,
	FPFBattleTargetSelection& OutSelection) const
{
	OutSelection = FPFBattleTargetSelection();

	if (!ValidateSelectionRequester(Requester))
	{
		return false;
	}

	if (!ActionTag.IsValid())
	{
		PF_LOG(TEXT("Target query failed: action tag is invalid"));
		return false;
	}

	const UPFAbilitySystemComponent* ASC =
		Requester->GetPFAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		PF_LOG(TEXT("Target query failed: ASC is unavailable. Requester=%s"),
			*GetNameSafe(Requester));
		return false;
	}

	FPFBattleTargetRule Rule;
	if (!ASC->GetBattleActionTargetRule(ActionTag, Rule))
	{
		return false;
	}

	CollectValidTargets(Requester, ActionTag, Rule, OutSelection.m_ValidTargets);
	if (OutSelection.m_ValidTargets.IsEmpty())
	{
		PF_LOG(TEXT("Target query failed: no valid targets. ActionTag=%s"),
			*ActionTag.ToString());
		return false;
	}

	OutSelection.m_TargetCount = Rule.m_Count;
	OutSelection.m_DefaultTarget = Rule.m_Count == EPFTargetCount::Single
		? OutSelection.m_ValidTargets[0].Get() : nullptr;
	return true;
}

bool UPFBattleRuntime::SelectBattleAction(
	APFBattleCharacterBase* Requester,
	FGameplayTag ActionTag)
{
	FPFBattleTargetSelection Selection;
	if (!GetValidTargetsAndDefaultTarget(Requester, ActionTag, Selection))
	{
		return false;
	}

	m_SelectedActionTag = ActionTag;
	m_TargetSelection = MoveTemp(Selection);
	BroadcastActionState(
		SIHGameplayTags::Message_Battle_ActionState_TargetSelection.GetTag());
	return true;
}

bool UPFBattleRuntime::SelectBattleTarget(
	APFBattleCharacterBase* Requester,
	APFBattleCharacterBase* Target)
{
	FPFBattleTargetSelection Selection;
	if (!GetValidTargetsAndDefaultTarget(Requester, m_SelectedActionTag, Selection))
	{
		return false;
	}

	if (Selection.m_TargetCount != EPFTargetCount::Single)
	{
		PF_LOG(TEXT("Cannot change an individual target for an All action"));
		return false;
	}

	if (!IsValid(Target) || !Selection.m_ValidTargets.Contains(Target))
	{
		PF_LOG(TEXT("Selected target is not a valid candidate. Target=%s"),
			*GetNameSafe(Target));
		return false;
	}

	Selection.m_DefaultTarget = Target;
	m_TargetSelection = MoveTemp(Selection);
	BroadcastActionState(
		SIHGameplayTags::Message_Battle_ActionState_TargetSelection.GetTag());
	return true;
}

bool UPFBattleRuntime::ConfirmBattleAction(APFBattleCharacterBase* Requester)
{
	FPFBattleTargetSelection CurrentSelection;
	if (!GetValidTargetsAndDefaultTarget(
		Requester, m_SelectedActionTag, CurrentSelection))
	{
		return false;
	}

	FPFBattleActionRequest Request;
	Request.m_Requester = Requester;
	Request.m_ActionTag = m_SelectedActionTag;

	if (CurrentSelection.m_TargetCount == EPFTargetCount::Single)
	{
		APFBattleCharacterBase* Target = m_TargetSelection.m_DefaultTarget.Get();
		if (!IsValid(Target) || !CurrentSelection.m_ValidTargets.Contains(Target))
		{
			PF_LOG(TEXT("Cannot confirm an action: selected target is unavailable"));
			return false;
		}

		Request.m_Target = Target;
	}

	return RequestBattleAction(Request);
}

bool UPFBattleRuntime::CancelBattleActionSelection(APFBattleCharacterBase* Requester)
{
	if (!ValidateSelectionRequester(Requester))
	{
		return false;
	}

	if (!m_SelectedActionTag.IsValid())
	{
		PF_LOG(TEXT("Cannot cancel an action: no action is selected"));
		return false;
	}

	ClearActionSelection();
	BroadcastActionState(
		SIHGameplayTags::Message_Battle_ActionState_ActionSelection.GetTag());
	return true;
}

bool UPFBattleRuntime::ValidateSelectionRequester(APFBattleCharacterBase* Requester) const
{
	if (m_FinalResult != EPFBattleResult::None
		|| !IsValid(m_TurnSystem)
		|| !m_TurnSystem->IsWaitingForActionRequest())
	{
		PF_LOG(TEXT("Battle is not waiting for action selection"));
		return false;
	}

	if (!IsValid(Requester)
		|| Requester != m_TurnSystem->GetCurrentActionOwner()
		|| !Requester->IsAlive())
	{
		PF_LOG(TEXT("Requester cannot select an action. Requester=%s"),
			*GetNameSafe(Requester));
		return false;
	}

	return true;
}

void UPFBattleRuntime::ClearActionSelection()
{
	m_SelectedActionTag = FGameplayTag();
	m_TargetSelection = FPFBattleTargetSelection();
}

void UPFBattleRuntime::CollectValidTargets(
	APFBattleCharacterBase* Requester,
	const FGameplayTag ActionTag,
	const FPFBattleTargetRule& Rule,
	TArray<TObjectPtr<APFBattleCharacterBase>>& OutTargets) const
{
	OutTargets.Reset();
	FPFBattleActionRequest CandidateRequest;
	CandidateRequest.m_Requester = Requester;
	CandidateRequest.m_ActionTag = ActionTag;

	if (Rule.m_Relation == EPFTargetRelation::Self)
	{
		CandidateRequest.m_Target = Requester;
		if (CandidateRequest.IsTargetSelectionValid(
			Rule.m_Relation, EPFTargetCount::Single))
		{
			OutTargets.Add(Requester);
		}
		return;
	}

	for (APFBattleCharacterBase* Participant : m_Participants)
	{
		CandidateRequest.m_Target = Participant;
		if (CandidateRequest.IsTargetSelectionValid(
			Rule.m_Relation, EPFTargetCount::Single))
		{
			OutTargets.Add(Participant);
		}
	}
}

bool UPFBattleRuntime::RequestBattleAction(
	const FPFBattleActionRequest& Request)
{
	if (m_FinalResult != EPFBattleResult::None)
	{
		PF_LOG(TEXT(
			"Cannot start an action after the battle has completed"));
		return false;
	}

	if (!IsValid(m_TurnSystem))
	{
		PF_LOG(TEXT("TurnSystem is unavailable"));
		return false;
	}

	if (!Request.IsValid()
		|| !IsValid(Request.m_Requester))
	{
		PF_LOG(TEXT("Battle action request is invalid"));
		return false;
	}

	if (Request.m_Requester
		!= m_TurnSystem->GetCurrentActionOwner())
	{
		PF_LOG(
			TEXT(
				"Battle action requester does not own the current "
				"action. Requester=%s, Current=%s"),
			*GetNameSafe(Request.m_Requester),
			*GetNameSafe(
				m_TurnSystem->GetCurrentActionOwner()));
		return false;
	}

	IPFBattleActionParticipantInterface* ActionParticipant =
		Cast<IPFBattleActionParticipantInterface>(
			Request.m_Requester);

	if (!ActionParticipant)
	{
		PF_LOG(TEXT(
			"Requester does not implement the action participant interface"));
		return false;
	}

	if (!m_TurnSystem->BeginActionProcessing(
		Request.m_Requester))
	{
		return false;
	}

	ClearActionSelection();
	BroadcastActionState(
		SIHGameplayTags::Message_Battle_ActionState_Processing.GetTag());

	if (!ActionParticipant->RequestBattleAction(Request))
	{
		m_TurnSystem->RestoreActionSelection(
			Request.m_Requester);
		BroadcastActionState(
			SIHGameplayTags::Message_Battle_ActionState_ActionSelection.GetTag());
		return false;
	}

	return true;
}

void UPFBattleRuntime::HandleBattleActionCompleted(
	AActor* ActionOwner)
{
	if (m_FinalResult != EPFBattleResult::None)
	{
		PF_LOG(TEXT(
			"Cannot complete an action after the battle has completed"));
		return;
	}

	if (!IsValid(m_TurnSystem))
	{
		PF_LOG(TEXT("TurnSystem is unavailable"));
		return;
	}

	m_TurnSystem->HandleBattleActionCompleted(ActionOwner);
}

void UPFBattleRuntime::HandleBattleProgressionReady()
{
	if (m_FinalResult != EPFBattleResult::None)
	{
		return;
	}

	EPFBattleResult BattleResult = EPFBattleResult::None;

	if (!DetermineBattleResult(BattleResult))
	{
		PF_LOG(TEXT("Failed to determine the battle result"));
		return;
	}

	if (BattleResult != EPFBattleResult::None)
	{
		CompleteBattle(BattleResult);
		return;
	}

	if (!m_TurnSystem->StartNextTurn())
	{
		PF_LOG(TEXT("Failed to start the next turn"));
		return;
	}

	RequestActionSelection();
}

void UPFBattleRuntime::CompleteBattle(
	EPFBattleResult ResultType)
{
	if (m_FinalResult != EPFBattleResult::None)
	{
		PF_LOG(TEXT("Battle has already completed"));
		return;
	}

	FPFBattleResult Result{};
	Result.m_CaseID = m_CaseID;
	Result.m_ResultType = ResultType;

	if (!Result.IsValid())
	{
		PF_LOG(TEXT("Battle result is invalid"));
		return;
	}

	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		PF_LOG(TEXT("GameplayMessageSubsystem is unavailable"));
		return;
	}

	m_FinalResult = ResultType;
	ClearActionSelection();
	UnbindActionCompletionDelegates();
	BroadcastActionState(
		SIHGameplayTags::Message_Battle_ActionState_Cleared.GetTag());

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		SIHGameplayTags::Message_Flow_Battle_Completed.GetTag(),
		Result);
}

void UPFBattleRuntime::BindActionCompletionDelegates()
{
	UnbindActionCompletionDelegates();

	for (APFBattleCharacterBase* Participant
		: m_Participants)
	{
		IPFBattleActionParticipantInterface* ActionParticipant =
			Cast<IPFBattleActionParticipantInterface>(
				Participant);

		if (!ActionParticipant)
		{
			continue;
		}

		const FDelegateHandle DelegateHandle =
			ActionParticipant
				->GetBattleActionCompletedDelegate()
				.AddUObject(
					this,
					&ThisClass::HandleBattleActionCompleted);

		m_ActionCompletionBindings.Add(
			Participant,
			DelegateHandle);
	}
}

void UPFBattleRuntime::UnbindActionCompletionDelegates()
{
	for (const TPair<TWeakObjectPtr<AActor>, FDelegateHandle>&
		Binding : m_ActionCompletionBindings)
	{
		AActor* Participant = Binding.Key.Get();

		if (!IsValid(Participant))
		{
			continue;
		}

		IPFBattleActionParticipantInterface* ActionParticipant =
			Cast<IPFBattleActionParticipantInterface>(
				Participant);

		if (ActionParticipant)
		{
			ActionParticipant
				->GetBattleActionCompletedDelegate()
				.Remove(Binding.Value);
		}
	}

	m_ActionCompletionBindings.Reset();
}

void UPFBattleRuntime::RequestActionSelection()
{
	if (m_FinalResult != EPFBattleResult::None)
	{
		PF_LOG(TEXT(
			"Cannot request action selection after battle completion"));
		return;
	}

	if (!IsValid(m_TurnSystem))
	{
		PF_LOG(TEXT("TurnSystem is unavailable"));
		return;
	}

	APFBattleCharacterBase* ActionOwner =
		m_TurnSystem->GetCurrentActionOwner();

	if (!IsValid(ActionOwner))
	{
		PF_LOG(TEXT("Current action owner is invalid"));
		return;
	}

	ClearActionSelection();
	BroadcastActionState(
		SIHGameplayTags::Message_Battle_ActionState_ActionSelection.GetTag());
}

void UPFBattleRuntime::BroadcastActionState(FGameplayTag Channel)
{
	FPFBattleActionStateMessage Message;

	if (m_FinalResult == EPFBattleResult::None
		&& IsValid(m_TurnSystem))
	{
		APFBattleCharacterBase* ActionOwner =
			m_TurnSystem->GetCurrentActionOwner();

		Message.m_ActionOwner = ActionOwner;
		Message.m_bCanPlayerSelectAction =
			IsValid(ActionOwner)
			&& ActionOwner->IsAlive()
			&& ActionOwner->GetBattleSide() == EPFBattleSide::Ally
			&& m_TurnSystem->IsWaitingForActionRequest();
	}

	if (Channel == SIHGameplayTags::Message_Battle_ActionState_TargetSelection.GetTag())
	{
		Message.m_SelectedActionTag = m_SelectedActionTag;
		Message.m_TargetSelection = m_TargetSelection;
	}

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(Channel, Message);
}

bool UPFBattleRuntime::DetermineBattleResult(
	EPFBattleResult& OutResult) const
{
	OutResult = EPFBattleResult::None;

	bool bHasAliveAlly = false;
	bool bHasAliveEnemy = false;

	for (APFBattleCharacterBase* Participant
		: m_Participants)
	{
		if (!IsValid(Participant))
		{
			PF_LOG(TEXT(
				"Cannot determine the result with an invalid participant"));
			return false;
		}

		if (!Participant->IsAlive())
		{
			continue;
		}

		switch (Participant->GetBattleSide())
		{
		case EPFBattleSide::Ally:
			bHasAliveAlly = true;
			break;

		case EPFBattleSide::Enemy:
			bHasAliveEnemy = true;
			break;

		default:
			PF_LOG(TEXT(
				"Cannot determine the result for an invalid battle side"));
			return false;
		}
	}

	if (!bHasAliveAlly)
	{
		OutResult = EPFBattleResult::Defeat;
		return true;
	}

	if (!bHasAliveEnemy)
	{
		OutResult = m_ResultOnVictory;
		return true;
	}

	return true;
}

void UPFBattleRuntime::Init()
{
	m_TurnSystem = NewObject<UPFTurnSystem>(this);

	if (IsValid(m_TurnSystem))
	{
		m_TurnSystem
			->GetBattleProgressionReadyDelegate()
			.AddUObject(
				this,
				&ThisClass::HandleBattleProgressionReady);
	}
	else
	{
		PF_LOG(TEXT("Failed to create TurnSystem"));
	}
}

void UPFBattleRuntime::Deinit()
{
	UnbindActionCompletionDelegates();

	if (IsValid(m_TurnSystem))
	{
		m_TurnSystem->Deinit();
	}
}
