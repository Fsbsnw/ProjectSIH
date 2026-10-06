#include "PFDeathAbilityBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFDeathParticipantInterface.h"
#include "Project_SIH/002_Systems/009_AbilitySystem/PFAbilitySystemComponent.h"

UPFDeathAbilityBase::UPFDeathAbilityBase()
{
	InstancingPolicy =
		EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UPFDeathAbilityBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		TriggerEventData);

	UPFAbilitySystemComponent* AbilitySystemComponent =
		ActorInfo
			? Cast<UPFAbilitySystemComponent>(
				ActorInfo->AbilitySystemComponent.Get())
			: nullptr;

	AActor* DeathOwner = GetAvatarActorFromActorInfo();
	IPFDeathParticipantInterface* DeathParticipant =
		Cast<IPFDeathParticipantInterface>(DeathOwner);

	if (!IsValid(AbilitySystemComponent)
		|| !DeathParticipant
		|| AbilitySystemComponent->GetGameplayTagCount(
			SIHGameplayTags::State_Death_Pending) == 0)
	{
		PF_LOG(TEXT("DeathAbility activation context is invalid"));
		Super::EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			false,
			true);
		return;
	}

	if (!AbilitySystemComponent
		->RemoveLooseGameplayTagIfExists(
			SIHGameplayTags::State_Death_Pending)
		|| !AbilitySystemComponent
			->AddLooseGameplayTagIfNone(
				SIHGameplayTags::State_Death))
	{
		PF_LOG(TEXT("Failed to enter the death state"));
		Super::EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			false,
			true);
		return;
	}

	DeathParticipant->NotifyDeathStarted();

	if (!IsValid(m_DeathMontage))
	{
		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			false,
			false);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::
			CreatePlayMontageAndWaitProxy(
				this,
				NAME_None,
				m_DeathMontage);

	if (!IsValid(MontageTask))
	{
		PF_LOG(TEXT("Failed to create the death montage task"));
		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			false,
			true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(
		this,
		&ThisClass::HandleDeathMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(
		this,
		&ThisClass::HandleDeathMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(
		this,
		&ThisClass::HandleDeathMontageInterrupted);
	MontageTask->ReadyForActivation();
}

void UPFDeathAbilityBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled)
{
	if (!IsEndAbilityValid(Handle, ActorInfo))
	{
		return;
	}

	AActor* DeathOwner = GetAvatarActorFromActorInfo();

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled);

	IPFDeathParticipantInterface* DeathParticipant =
		Cast<IPFDeathParticipantInterface>(DeathOwner);

	if (DeathParticipant)
	{
		DeathParticipant->NotifyDeathFinished();
	}
	else
	{
		PF_LOG(TEXT("Cannot notify death completion: participant interface is unavailable. DeathOwner=%s"),
			*GetNameSafe(DeathOwner));
	}
}

void UPFDeathAbilityBase::HandleDeathMontageCompleted()
{
	EndAbility(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActorInfo(),
		GetCurrentActivationInfo(),
		false,
		false);
}

void UPFDeathAbilityBase::HandleDeathMontageInterrupted()
{
	EndAbility(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActorInfo(),
		GetCurrentActivationInfo(),
		false,
		true);
}
