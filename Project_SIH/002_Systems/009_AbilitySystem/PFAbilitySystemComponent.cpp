#include "PFAbilitySystemComponent.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/009_AbilitySystem/PFBattleActionAbilityInterface.h"
#include "Project_SIH/002_Systems/009_AbilitySystem/PFDeathAbilityBase.h"

bool UPFAbilitySystemComponent::ActivateBattleAction(
	const FPFBattleActionRequest& Request)
{
	if (!Request.IsValid()
		|| !IsValid(Request.m_Requester)
		|| Request.m_Requester != GetAvatarActor())
	{
		PF_LOG(TEXT("Battle action request is invalid"));
		return false;
	}

	const FGameplayAbilitySpecHandle* AbilityHandle =
		m_BattleActionAbilityHandles.Find(
			Request.m_ActionTag);

	if (!AbilityHandle)
	{
		PF_LOG(
			TEXT(
				"No battle action ability is mapped. "
				"ActionTag=%s"),
			*Request.m_ActionTag.ToString());
		return false;
	}

	const FGameplayAbilitySpec* AbilitySpec =
		FindAbilitySpecFromHandle(*AbilityHandle);

	const UObject* AbilityObject =
		AbilitySpec ? AbilitySpec->Ability.Get() : nullptr;
	const IPFBattleActionAbilityInterface* ActionAbility =
		Cast<IPFBattleActionAbilityInterface>(AbilityObject);

	if (!ActionAbility)
	{
		PF_LOG(TEXT("Battle action interface is unavailable. ActionTag=%s"),
			*Request.m_ActionTag.ToString());
		return false;
	}

	if (!ActionAbility->CanActivateBattleAction(Request))
	{
		return false;
	}

	FGameplayEventData EventData;
	EventData.EventTag = Request.m_ActionTag;
	EventData.Instigator = Request.m_Requester;
	EventData.Target = Request.m_Target;

	if (!InternalTryActivateAbility(
		*AbilityHandle,
		FPredictionKey(),
		nullptr,
		nullptr,
		&EventData))
	{
		PF_LOG(TEXT("Failed to activate battle action. ActionTag=%s, Requester=%s"),
			*Request.m_ActionTag.ToString(), *GetNameSafe(Request.m_Requester));
		return false;
	}

	return true;
}

bool UPFAbilitySystemComponent::GetBattleActionTargetRule(
	const FGameplayTag ActionTag,
	FPFBattleTargetRule& OutRule) const
{
	OutRule = FPFBattleTargetRule();
	const FGameplayAbilitySpecHandle* AbilityHandle =
		m_BattleActionAbilityHandles.Find(ActionTag);

	if (!AbilityHandle)
	{
		PF_LOG(TEXT("Target rule query failed: action is not granted. ActionTag=%s"),
			*ActionTag.ToString());
		return false;
	}

	const FGameplayAbilitySpec* AbilitySpec =
		FindAbilitySpecFromHandle(*AbilityHandle);
	const UObject* AbilityObject =
		AbilitySpec ? AbilitySpec->Ability.Get() : nullptr;
	const IPFBattleActionAbilityInterface* ActionAbility =
		Cast<IPFBattleActionAbilityInterface>(AbilityObject);

	if (!ActionAbility)
	{
		PF_LOG(TEXT("Battle action interface is unavailable. ActionTag=%s"),
			*ActionTag.ToString());
		return false;
	}

	OutRule = ActionAbility->GetTargetRule();
	if (!OutRule.IsValid())
	{
		PF_LOG(TEXT("Self targeting requires Single target count. ActionTag=%s"),
			*ActionTag.ToString());
		return false;
	}

	return true;
}

bool UPFAbilitySystemComponent::HasBattleAction(
	const FGameplayTag ActionTag) const
{
	return m_BattleActionAbilityHandles.Contains(
		ActionTag);
}

bool UPFAbilitySystemComponent::ActivateDeathAbility()
{
	if (!m_DeathAbilityHandle.IsValid())
	{
		PF_LOG(TEXT("No DeathAbility is granted"));
		return false;
	}

	if (!TryActivateAbility(m_DeathAbilityHandle))
	{
		PF_LOG(TEXT("Failed to activate DeathAbility. Avatar=%s"),
			*GetNameSafe(GetAvatarActor()));
		return false;
	}

	return true;
}

bool UPFAbilitySystemComponent::HasDeathAbility() const
{
	return m_DeathAbilityHandle.IsValid();
}

bool UPFAbilitySystemComponent::AddLooseGameplayTagIfNone(
	const FGameplayTag GameplayTag)
{
	if (!GameplayTag.IsValid())
	{
		PF_LOG(
			TEXT(
				"Failed to add loose gameplay tag. "
				"GameplayTag is invalid"));
		return false;
	}

	const int32 GameplayTagCount =
		GetGameplayTagCount(GameplayTag);

	if (GameplayTagCount > 0)
	{
		PF_LOG(
			TEXT(
				"Failed to add loose gameplay tag. "
				"GameplayTag already exists. "
				"GameplayTag=%s, Count=%d"),
			*GameplayTag.ToString(),
			GameplayTagCount);
		return false;
	}

	AddLooseGameplayTag(GameplayTag);

	return true;
}

bool UPFAbilitySystemComponent::RemoveLooseGameplayTagIfExists(
	const FGameplayTag GameplayTag)
{
	if (!GameplayTag.IsValid())
	{
		PF_LOG(
			TEXT(
				"Failed to remove loose gameplay tag. "
				"GameplayTag is invalid"));
		return false;
	}

	const int32 GameplayTagCount =
		GetGameplayTagCount(GameplayTag);

	if (GameplayTagCount == 0)
	{
		PF_LOG(
			TEXT(
				"Failed to remove loose gameplay tag. "
				"GameplayTag does not exist. "
				"GameplayTag=%s"),
			*GameplayTag.ToString());
		return false;
	}

	if (GameplayTagCount > 1)
	{
		PF_LOG(
			TEXT(
				"Failed to remove loose gameplay tag. "
				"GameplayTag count is greater than one. "
				"GameplayTag=%s, Count=%d"),
			*GameplayTag.ToString(),
			GameplayTagCount);
		return false;
	}

	RemoveLooseGameplayTag(GameplayTag);

	return true;
}

void UPFAbilitySystemComponent::OnGiveAbility(
	FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	const UObject* AbilityObject = AbilitySpec.Ability.Get();
	const IPFBattleActionAbilityInterface* BattleActionAbility =
		Cast<IPFBattleActionAbilityInterface>(AbilityObject);

	if (!BattleActionAbility)
	{
		if (!AbilityObject || !AbilityObject->IsA<UPFDeathAbilityBase>())
		{
			return;
		}

		if (m_DeathAbilityHandle.IsValid()
			&& m_DeathAbilityHandle != AbilitySpec.Handle)
		{
			PF_LOG(
				TEXT(
					"DeathAbility is duplicated. Ability=%s"),
				*GetNameSafe(AbilitySpec.Ability.Get()));
			return;
		}

		m_DeathAbilityHandle = AbilitySpec.Handle;
		return;
	}

	const FGameplayTag ActionTag =
		BattleActionAbility->GetActionTag();

	if (!ActionTag.IsValid())
	{
		PF_LOG(
			TEXT(
				"Battle action ability has no valid ActionTag. "
				"Ability=%s"),
			*GetNameSafe(AbilitySpec.Ability.Get()));
		return;
	}

	const FGameplayAbilitySpecHandle* ExistingHandle =
		m_BattleActionAbilityHandles.Find(ActionTag);

	if (ExistingHandle)
	{
		if (*ExistingHandle == AbilitySpec.Handle)
		{
			return;
		}

		PF_LOG(
			TEXT(
				"Battle action ActionTag is duplicated. "
				"ActionTag=%s, Ability=%s"),
			*ActionTag.ToString(),
			*GetNameSafe(AbilitySpec.Ability.Get()));
		return;
	}

	m_BattleActionAbilityHandles.Add(
		ActionTag,
		AbilitySpec.Handle);
}

void UPFAbilitySystemComponent::OnRemoveAbility(
	FGameplayAbilitySpec& AbilitySpec)
{
	const UObject* AbilityObject = AbilitySpec.Ability.Get();
	const IPFBattleActionAbilityInterface* BattleActionAbility =
		Cast<IPFBattleActionAbilityInterface>(AbilityObject);

	if (BattleActionAbility)
	{
		const FGameplayTag ActionTag =
			BattleActionAbility->GetActionTag();

		const FGameplayAbilitySpecHandle* MappedHandle =
			m_BattleActionAbilityHandles.Find(ActionTag);

		if (MappedHandle
			&& *MappedHandle == AbilitySpec.Handle)
		{
			m_BattleActionAbilityHandles.Remove(
				ActionTag);
		}
	}

	if (AbilityObject && AbilityObject->IsA<UPFDeathAbilityBase>()
		&& m_DeathAbilityHandle == AbilitySpec.Handle)
	{
		m_DeathAbilityHandle = FGameplayAbilitySpecHandle();
	}

	Super::OnRemoveAbility(AbilitySpec);
}
