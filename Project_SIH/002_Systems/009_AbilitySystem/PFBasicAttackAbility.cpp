#include "PFBasicAttackAbility.h"
#include "PFEffectBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayEffect.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"

UPFBasicAttackAbility::UPFBasicAttackAbility()
{
	InstancingPolicy =
		EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

FGameplayTag UPFBasicAttackAbility::GetActionTag() const
{
	return SIHGameplayTags::Action_Ability_BasicAttack.GetTag();
}

bool UPFBasicAttackAbility::CanActivateBattleAction(
	const FPFBattleActionRequest& Request) const
{
	if (!Super::CanActivateBattleAction(Request))
	{
		return false;
	}

	const FPFBattleTargetRule Rule = GetTargetRule();
	if (!Rule.IsValid())
	{
		PF_LOG(TEXT("Basic attack target rule is invalid"));
		return false;
	}

	if (!Request.IsTargetSelectionValid(Rule.m_Relation, Rule.m_Count))
	{
		PF_LOG(TEXT("Basic attack target selection is invalid. Requester=%s, Target=%s"),
			*GetNameSafe(Request.m_Requester), *GetNameSafe(Request.m_Target));
		return false;
	}

	if (!IsValid(m_AttackMontage))
	{
		PF_LOG(TEXT("Basic attack montage is unavailable. Ability=%s"),
			*GetNameSafe(this));
		return false;
	}

	if (!m_DamageEffect)
	{
		PF_LOG(TEXT("Basic attack damage effect is unavailable. Ability=%s"),
			*GetNameSafe(this));
		return false;
	}

	const IAbilitySystemInterface* TargetAbilitySystem =
		Cast<IAbilitySystemInterface>(Request.m_Target);

	if (!TargetAbilitySystem
		|| !IsValid(TargetAbilitySystem->GetAbilitySystemComponent()))
	{
		PF_LOG(TEXT("Basic attack target ASC is unavailable. Target=%s"),
			*GetNameSafe(Request.m_Target));
		return false;
	}

	return true;
}

void UPFBasicAttackAbility::ActivateAbility(
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

	m_TargetASC.Reset();

	const AActor* SelectedTarget =
		TriggerEventData
			? TriggerEventData->Target.Get()
			: nullptr;

	const IAbilitySystemInterface* TargetAbilitySystem =
		Cast<IAbilitySystemInterface>(SelectedTarget);

	if (!TargetAbilitySystem
		|| !IsValid(m_AttackMontage)
		|| !m_DamageEffect)
	{
		PF_LOG(TEXT("Basic attack activation context is invalid"));
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	m_TargetASC =
		TargetAbilitySystem->GetAbilitySystemComponent();

	if (!m_TargetASC.IsValid())
	{
		PF_LOG(TEXT("Basic attack target ASC is unavailable"));
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::
			CreatePlayMontageAndWaitProxy(
				this,
				NAME_None,
				m_AttackMontage);

	UAbilityTask_WaitGameplayEvent* HitTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			SIHGameplayTags::Event_Combat_BasicAttack_Hit,
			nullptr,
			false,
			true);

	if (!IsValid(MontageTask) || !IsValid(HitTask))
	{
		PF_LOG(TEXT("Failed to create basic attack ability tasks"));
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	HitTask->EventReceived.AddDynamic(
		this,
		&ThisClass::HandleHitEvent);

	MontageTask->OnCompleted.AddDynamic(
		this,
		&ThisClass::HandleMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(
		this,
		&ThisClass::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(
		this,
		&ThisClass::HandleMontageInterrupted);

	HitTask->ReadyForActivation();
	MontageTask->ReadyForActivation();
}

void UPFBasicAttackAbility::HandleHitEvent(
	FGameplayEventData)
{
	UAbilitySystemComponent* SourceASC =
		GetAbilitySystemComponentFromActorInfo();

	UAbilitySystemComponent* TargetASC =
		m_TargetASC.Get();

	if (!IsValid(SourceASC)
		|| !IsValid(TargetASC)
		|| !m_DamageEffect)
	{
		PF_LOG(TEXT("Basic attack damage context is invalid"));
		return;
	}

	const FGameplayEffectSpecHandle DamageSpec =
		SourceASC->MakeOutgoingSpec(
			m_DamageEffect.Get(),
			GetAbilityLevel(),
			SourceASC->MakeEffectContext());

	if (!DamageSpec.IsValid())
	{
		PF_LOG(TEXT("Failed to create basic attack damage effect"));
		return;
	}

	SourceASC->ApplyGameplayEffectSpecToTarget(
		*DamageSpec.Data.Get(),
		TargetASC);
}

void UPFBasicAttackAbility::HandleMontageCompleted()
{
	EndAbility(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActorInfo(),
		GetCurrentActivationInfo(),
		false,
		false);
}

void UPFBasicAttackAbility::HandleMontageInterrupted()
{
	EndAbility(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActorInfo(),
		GetCurrentActivationInfo(),
		false,
		true);
}

void UPFBasicAttackAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled)
{
	m_TargetASC.Reset();

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled);
}
