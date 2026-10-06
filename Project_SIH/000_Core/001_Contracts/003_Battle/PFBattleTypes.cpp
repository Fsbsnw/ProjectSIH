#include "PFBattleTypes.h"

#include "GameFramework/Actor.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFBattleParticipantInterface.h"

bool FPFBattleActionRequest::IsTargetSelectionValid(
	const EPFTargetRelation Relation,
	const EPFTargetCount Count) const
{
	if (!IsValid() || !::IsValid(m_Requester))
	{
		return false;
	}

	const IPFBattleParticipantInterface* Requester =
		Cast<IPFBattleParticipantInterface>(m_Requester);

	if (!Requester
		|| !Requester->IsAlive()
		|| Requester->GetBattleSide() == EPFBattleSide::None)
	{
		return false;
	}

	if ((Relation != EPFTargetRelation::Self
			&& Relation != EPFTargetRelation::SameSide
			&& Relation != EPFTargetRelation::OpposingSide)
		|| (Count != EPFTargetCount::Single
			&& Count != EPFTargetCount::All))
	{
		return false;
	}

	if (Relation == EPFTargetRelation::Self)
	{
		return Count == EPFTargetCount::Single
			&& (!m_Target || m_Target == m_Requester);
	}

	if (Count == EPFTargetCount::All)
	{
		// All 대상의 실제 후보 목록은 별도 선정 단계에서 결정한다.
		return m_Target == nullptr;
	}

	if (Count != EPFTargetCount::Single
		|| !::IsValid(m_Target)
		|| m_Requester->GetWorld() != m_Target->GetWorld())
	{
		return false;
	}

	const IPFBattleParticipantInterface* Target =
		Cast<IPFBattleParticipantInterface>(m_Target);

	if (!Target
		|| !Target->IsAlive()
		|| Target->GetBattleSide() == EPFBattleSide::None)
	{
		return false;
	}

	const bool bSameSide =
		Requester->GetBattleSide() == Target->GetBattleSide();

	return Relation == EPFTargetRelation::SameSide
		? bSameSide
		: Relation == EPFTargetRelation::OpposingSide
			&& !bSameSide;
}
