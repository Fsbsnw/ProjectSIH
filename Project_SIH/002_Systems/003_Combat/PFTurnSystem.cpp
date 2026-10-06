#include "PFTurnSystem.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/003_Combat/001_Characters/PFBattleCharacterBase.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFDeathParticipantInterface.h"

void UPFTurnSystem::Deinit()
{
	UnbindDeathDelegates();
	m_OnBattleProgressionReady.Clear();
}

bool UPFTurnSystem::StartManagingTurns(
	const TArray<TObjectPtr<APFBattleCharacterBase>>&
		Participants)
{
	UnbindDeathDelegates();
	m_Participants = Participants;
	m_RegularTurnOrder.Reset();
	m_CurrentTurnIndex = INDEX_NONE;
	m_State = EPFTurnFlowState::NotStarted;
	m_ActiveDeaths.Reset();
	m_bStartingPendingDeaths = false;

	if (!BuildRegularTurnOrder())
	{
		PF_LOG(TEXT("Failed to build the initial turn order"));
		m_Participants.Reset();
		return false;
	}

	m_CurrentTurnIndex = 0;
	BindDeathDelegates();
	BeginCurrentRegularTurn();

	return true;
}

bool UPFTurnSystem::BeginActionProcessing(
	AActor* ActionOwner)
{
	if (m_State != EPFTurnFlowState::ActionSelection)
	{
		PF_LOG(
			TEXT(
				"Cannot start an action in the current "
				"turn state. State=%d"),
			static_cast<uint8>(m_State));
		return false;
	}

	APFBattleCharacterBase* CurrentActionOwner =
		GetCurrentActionOwner();

	if (!IsValid(ActionOwner)
		|| ActionOwner != CurrentActionOwner)
	{
		PF_LOG(
			TEXT(
				"Action owner does not match the current "
				"turn owner. Requested=%s, Current=%s"),
			*GetNameSafe(ActionOwner),
			*GetNameSafe(CurrentActionOwner));
		return false;
	}

	m_State = EPFTurnFlowState::ActionProcessing;
	return true;
}

void UPFTurnSystem::RestoreActionSelection(
	AActor* ActionOwner)
{
	if (m_State != EPFTurnFlowState::ActionProcessing)
	{
		PF_LOG(
			TEXT(
				"Cannot restore action selection in the current "
				"turn state. State=%d"),
			static_cast<uint8>(m_State));
		return;
	}

	APFBattleCharacterBase* CurrentActionOwner =
		GetCurrentActionOwner();

	if (!IsValid(ActionOwner)
		|| ActionOwner != CurrentActionOwner)
	{
		PF_LOG(
			TEXT(
				"Action owner does not match the current "
				"turn owner. Requested=%s, Current=%s"),
			*GetNameSafe(ActionOwner),
			*GetNameSafe(CurrentActionOwner));
		return;
	}

	m_State = EPFTurnFlowState::ActionSelection;
}

void UPFTurnSystem::HandleBattleActionCompleted(
	AActor* ActionOwner)
{
	if (m_State != EPFTurnFlowState::ActionProcessing)
	{
		PF_LOG(
			TEXT(
				"Cannot complete an action in the current "
				"turn state. State=%d"),
			static_cast<uint8>(m_State));
		return;
	}

	APFBattleCharacterBase* CurrentActionOwner =
		GetCurrentActionOwner();

	if (!IsValid(ActionOwner)
		|| ActionOwner != CurrentActionOwner)
	{
		PF_LOG(
			TEXT(
				"Action owner does not match the current "
				"turn owner. Requested=%s, Current=%s"),
			*GetNameSafe(ActionOwner),
			*GetNameSafe(CurrentActionOwner));
		return;
	}

	FlushPendingDeaths();
}

bool UPFTurnSystem::StartNextTurn()
{
	if (m_State != EPFTurnFlowState::TurnEndProcessing)
	{
		PF_LOG(
			TEXT(
				"Cannot start the next turn in the current "
				"turn state. State=%d"),
			static_cast<uint8>(m_State));
		return false;
	}

	for (int32 NextIndex = m_CurrentTurnIndex + 1;
		NextIndex < m_RegularTurnOrder.Num();
		++NextIndex)
	{
		APFBattleCharacterBase* NextParticipant =
			m_RegularTurnOrder[NextIndex];

		if (!IsValid(NextParticipant))
		{
			PF_LOG(
				TEXT(
					"Turn participant became invalid while "
					"advancing the current turn order"));

			m_RegularTurnOrder.Reset();
			m_CurrentTurnIndex = INDEX_NONE;
			m_State = EPFTurnFlowState::NotStarted;
			return false;
		}

		if (!NextParticipant->IsAlive())
		{
			continue;
		}

		m_CurrentTurnIndex = NextIndex;
		BeginCurrentRegularTurn();
		return true;
	}

	if (!BuildRegularTurnOrder())
	{
		m_CurrentTurnIndex = INDEX_NONE;
		return false;
	}

	m_CurrentTurnIndex = 0;
	BeginCurrentRegularTurn();
	return true;
}

APFBattleCharacterBase*
UPFTurnSystem::GetCurrentActionOwner() const
{
	if ((m_State != EPFTurnFlowState::ActionSelection
			&& m_State != EPFTurnFlowState::ActionProcessing)
		|| !m_RegularTurnOrder.IsValidIndex(
			m_CurrentTurnIndex))
	{
		return nullptr;
	}

	APFBattleCharacterBase* CurrentParticipant =
		m_RegularTurnOrder[m_CurrentTurnIndex];

	return IsValid(CurrentParticipant)
		? CurrentParticipant
		: nullptr;
}

bool UPFTurnSystem::IsWaitingForActionRequest() const
{
	return m_State == EPFTurnFlowState::ActionSelection
		&& GetCurrentActionOwner() != nullptr;
}

FPFOnBattleProgressionReady&
UPFTurnSystem::GetBattleProgressionReadyDelegate()
{
	return m_OnBattleProgressionReady;
}

bool UPFTurnSystem::BuildRegularTurnOrder()
{
	m_RegularTurnOrder.Reset();

	for (APFBattleCharacterBase* Participant
		: m_Participants)
	{
		if (!IsValid(Participant))
		{
			PF_LOG(
				TEXT(
					"Turn participant is invalid while "
					"building the turn order"));
			return false;
		}

		if (!Participant->IsAlive())
		{
			continue;
		}

		m_RegularTurnOrder.Add(Participant);
	}

	if (m_RegularTurnOrder.IsEmpty())
	{
		PF_LOG(TEXT("No alive participant remains"));
		return false;
	}

	m_RegularTurnOrder.StableSort(
		[](const APFBattleCharacterBase& Left,
			const APFBattleCharacterBase& Right)
		{
			return Left.GetTurnSpeed()
				> Right.GetTurnSpeed();
		});

	return true;
}

void UPFTurnSystem::BeginCurrentRegularTurn()
{
	if (!m_RegularTurnOrder.IsValidIndex(
			m_CurrentTurnIndex))
	{
		PF_LOG(
			TEXT(
				"Current turn index is invalid. Index=%d, "
				"TurnOrderCount=%d"),
			m_CurrentTurnIndex,
			m_RegularTurnOrder.Num());
		return;
	}

	m_State = EPFTurnFlowState::TurnStartProcessing;
	m_State = EPFTurnFlowState::ActionSelection;
}

void UPFTurnSystem::FlushPendingDeaths()
{
	m_bStartingPendingDeaths = true;

	for (APFBattleCharacterBase* Participant
		: m_Participants)
	{
		IPFDeathParticipantInterface* DeathParticipant =
			Cast<IPFDeathParticipantInterface>(Participant);

		if (!DeathParticipant
			|| !DeathParticipant->HasPendingDeath())
		{
			continue;
		}

		if (!DeathParticipant->StartPendingDeath())
		{
			PF_LOG(
				TEXT(
					"Failed to start pending death. "
					"Participant=%s"),
				*GetNameSafe(Participant));
		}
	}

	m_bStartingPendingDeaths = false;
	TryFinishActionProcessing();
}

void UPFTurnSystem::HandleDeathStarted(AActor* Participant)
{
	if (!IsValid(Participant))
	{
		PF_LOG(TEXT("Death started for an invalid participant"));
		return;
	}

	m_ActiveDeaths.Add(Participant);
}

void UPFTurnSystem::HandleDeathFinished(AActor* Participant)
{
	if (!IsValid(Participant))
	{
		PF_LOG(TEXT("Death finished for an invalid participant"));
		return;
	}

	m_ActiveDeaths.Remove(Participant);
	TryFinishActionProcessing();
}

void UPFTurnSystem::TryFinishActionProcessing()
{
	if (m_State != EPFTurnFlowState::ActionProcessing
		|| m_bStartingPendingDeaths
		|| !m_ActiveDeaths.IsEmpty())
	{
		return;
	}

	for (APFBattleCharacterBase* Participant
		: m_Participants)
	{
		IPFDeathParticipantInterface* DeathParticipant =
			Cast<IPFDeathParticipantInterface>(Participant);

		if (DeathParticipant
			&& DeathParticipant->HasPendingDeath())
		{
			return;
		}
	}

	m_State = EPFTurnFlowState::TurnEndProcessing;
	m_OnBattleProgressionReady.Broadcast();
}

void UPFTurnSystem::BindDeathDelegates()
{
	UnbindDeathDelegates();

	for (APFBattleCharacterBase* Participant
		: m_Participants)
	{
		IPFDeathParticipantInterface* DeathParticipant =
			Cast<IPFDeathParticipantInterface>(Participant);

		if (!DeathParticipant)
		{
			continue;
		}

		m_DeathStartedBindings.Add(
			Participant,
			DeathParticipant->GetDeathStartedDelegate()
				.AddUObject(
					this,
					&ThisClass::HandleDeathStarted));

		m_DeathFinishedBindings.Add(
			Participant,
			DeathParticipant->GetDeathFinishedDelegate()
				.AddUObject(
					this,
					&ThisClass::HandleDeathFinished));
	}
}

void UPFTurnSystem::UnbindDeathDelegates()
{
	for (const TPair<TWeakObjectPtr<AActor>, FDelegateHandle>&
		Binding : m_DeathStartedBindings)
	{
		AActor* Participant = Binding.Key.Get();
		IPFDeathParticipantInterface* DeathParticipant =
			Cast<IPFDeathParticipantInterface>(Participant);

		if (DeathParticipant)
		{
			DeathParticipant->GetDeathStartedDelegate()
				.Remove(Binding.Value);
		}
	}

	for (const TPair<TWeakObjectPtr<AActor>, FDelegateHandle>&
		Binding : m_DeathFinishedBindings)
	{
		AActor* Participant = Binding.Key.Get();
		IPFDeathParticipantInterface* DeathParticipant =
			Cast<IPFDeathParticipantInterface>(Participant);

		if (DeathParticipant)
		{
			DeathParticipant->GetDeathFinishedDelegate()
				.Remove(Binding.Value);
		}
	}

	m_DeathStartedBindings.Reset();
	m_DeathFinishedBindings.Reset();
}
