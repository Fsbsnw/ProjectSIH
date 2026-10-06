#include "PFBattleActionAbilityBase.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFBattleActionParticipantInterface.h"

FGameplayTag
UPFBattleActionAbilityBase::GetActionTag() const
{
	return FGameplayTag();
}

FPFBattleTargetRule UPFBattleActionAbilityBase::GetTargetRule() const
{
	return m_TargetRule;
}

bool UPFBattleActionAbilityBase::CanActivateBattleAction(
	const FPFBattleActionRequest& Request) const
{
	if (!Request.IsValid() || Request.m_ActionTag != GetActionTag())
	{
		PF_LOG(TEXT("Battle action request does not match its ability. ActionTag=%s, Ability=%s"),
			*Request.m_ActionTag.ToString(), *GetNameSafe(this));
		return false;
	}

	return true;
}

void UPFBattleActionAbilityBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled)
{
	const bool bCanEndAbility =
		IsEndAbilityValid(Handle, ActorInfo);

	AActor* ActionOwner =
		GetAvatarActorFromActorInfo();

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled);

	if (!bCanEndAbility)
	{
		return;
	}

	IPFBattleActionParticipantInterface* Participant =
		Cast<IPFBattleActionParticipantInterface>(
			ActionOwner);

	if (Participant)
	{
		Participant->NotifyBattleActionCompleted();
	}
	else
	{
		PF_LOG(TEXT("Cannot notify action completion: participant interface is unavailable. ActionOwner=%s"),
			*GetNameSafe(ActionOwner));
	}
}
