#include "PFBattleCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Contracts/004_Character/PFCharacterStatTypes.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/003_Combat/002_Attributes/PFBattleAttributeSet.h"
#include "Project_SIH/002_Systems/009_AbilitySystem/PFAbilityBase.h"
#include "Project_SIH/002_Systems/009_AbilitySystem/PFAbilitySystemComponent.h"
#include "Project_SIH/002_Systems/009_AbilitySystem/PFBattleActionAbilityInterface.h"
#include "Project_SIH/002_Systems/009_AbilitySystem/PFDeathAbilityBase.h"

namespace
{
	bool ValidateInitialBattleAttributes(
		const FPFCharacterStatValues& InitialStats)
	{
		for (const FPFStatRowBinding& Binding
			: FPFCharacterStatValues::GetRowBindings())
		{
			if (InitialStats.*Binding.m_ValueMember < 0.0f)
			{
				return false;
			}
		}

		return true;
	}
}

APFBattleCharacterBase::APFBattleCharacterBase()
{
	m_AbilitySystemComponent =
		CreateDefaultSubobject<UPFAbilitySystemComponent>(
			TEXT("AbilitySystemComponent"));

	m_BattleAttributeSet =
		CreateDefaultSubobject<UPFBattleAttributeSet>(
			TEXT("BattleAttributeSet"));
}

void APFBattleCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	m_AbilitySystemComponent->InitAbilityActorInfo(
		this,
		this);

	m_AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(
			UPFBattleAttributeSet::GetHPAttribute())
		.AddUObject(
			this,
			&ThisClass::HandleHPChanged);
}

UAbilitySystemComponent*
APFBattleCharacterBase::GetAbilitySystemComponent() const
{
	return m_AbilitySystemComponent;
}

UPFAbilitySystemComponent*
APFBattleCharacterBase::GetPFAbilitySystemComponent() const
{
	return m_AbilitySystemComponent;
}

EPFBattleSide APFBattleCharacterBase::GetBattleSide() const
{
	return EPFBattleSide::None;
}

bool APFBattleCharacterBase::IsAlive() const
{
	return !m_AbilitySystemComponent
		->HasMatchingGameplayTag(
			SIHGameplayTags::State_Death);
}

float APFBattleCharacterBase::GetTurnSpeed() const
{
	return m_BattleAttributeSet->GetTurnSpeed();
}

bool APFBattleCharacterBase::RequestBattleAction(
	const FPFBattleActionRequest& Request)
{
	if (Request.m_Requester != this)
	{
		PF_LOG(
			TEXT(
				"Battle action requester does not match the "
				"character. Requester=%s, Character=%s"),
			*GetNameSafe(Request.m_Requester),
			*GetNameSafe(this));
		return false;
	}

	if (!IsValid(m_AbilitySystemComponent))
	{
		PF_LOG(TEXT("AbilitySystemComponent is unavailable"));
		return false;
	}

	return m_AbilitySystemComponent
		->ActivateBattleAction(Request);
}

void APFBattleCharacterBase::NotifyBattleActionCompleted()
{
	m_OnBattleActionCompleted.Broadcast(this);
}

FPFOnBattleActionCompleted&
APFBattleCharacterBase::GetBattleActionCompletedDelegate()
{
	return m_OnBattleActionCompleted;
}

bool APFBattleCharacterBase::HasPendingDeath() const
{
	return IsValid(m_AbilitySystemComponent)
		&& m_AbilitySystemComponent->GetGameplayTagCount(
			SIHGameplayTags::State_Death_Pending) > 0;
}

bool APFBattleCharacterBase::StartPendingDeath()
{
	if (!HasPendingDeath())
	{
		PF_LOG(TEXT("Character has no pending death"));
		return false;
	}

	if (!IsValid(m_AbilitySystemComponent))
	{
		PF_LOG(TEXT("AbilitySystemComponent is unavailable"));
		return false;
	}

	return m_AbilitySystemComponent->ActivateDeathAbility();
}

void APFBattleCharacterBase::NotifyDeathStarted()
{
	m_OnDeathStarted.Broadcast(this);
}

void APFBattleCharacterBase::NotifyDeathFinished()
{
	m_OnDeathFinished.Broadcast(this);
}

FPFOnDeathStarted&
APFBattleCharacterBase::GetDeathStartedDelegate()
{
	return m_OnDeathStarted;
}

FPFOnDeathFinished&
APFBattleCharacterBase::GetDeathFinishedDelegate()
{
	return m_OnDeathFinished;
}

bool APFBattleCharacterBase::TryInitializeBattleAttributes(
	const FPFCharacterStatValues& InitialStats)
{
	if (!ValidateInitialBattleAttributes(InitialStats))
	{
		PF_LOG(
			TEXT(
				"Initial battle attributes contain "
				"a negative value"));
		return false;
	}

	m_BattleAttributeSet->InitAttack(
		InitialStats.m_Attack);

	m_BattleAttributeSet->InitPhysicalDefense(
		InitialStats.m_PhysicalDefense);

	m_BattleAttributeSet->InitMagicalDefense(
		InitialStats.m_MagicalDefense);

	m_BattleAttributeSet->InitTurnSpeed(
		InitialStats.m_TurnSpeed);

	m_BattleAttributeSet->InitMaxHP(
		InitialStats.m_MaxHP);

	m_BattleAttributeSet->InitHP(
		InitialStats.m_MaxHP);

	m_BattleAttributeSet->InitMaxUltimateGauge(
		InitialStats.m_MaxUltimateGauge);

	m_BattleAttributeSet->InitUltimateGauge(0.0f);

	return true;
}

bool APFBattleCharacterBase::TryGrantInitialAbilities(
	const TArray<TSubclassOf<UPFAbilityBase>>& AbilityClasses)
{
	if (!ValidateInitialAbilityClasses(AbilityClasses))
	{
		return false;
	}

	for (const TSubclassOf<UPFAbilityBase> AbilityClass
		: AbilityClasses)
	{
		GrantInitialAbility(AbilityClass);
	}

	return true;
}

bool APFBattleCharacterBase::ValidateInitialAbilityClasses(
	const TArray<TSubclassOf<UPFAbilityBase>>& AbilityClasses)
	const
{
	TSet<const UClass*> SeenAbilityClasses;
	TSet<FGameplayTag> SeenActionTags;
	bool bHasDeathAbility = false;

	for (int32 Index = 0;
		Index < AbilityClasses.Num();
		++Index)
	{
		const TSubclassOf<UPFAbilityBase> AbilityClass =
			AbilityClasses[Index];

		const UClass* LoadedClass =
			AbilityClass.Get();

		if (!IsValid(LoadedClass))
		{
			PF_LOG(
				TEXT(
					"Initial ability class is invalid. "
					"Index=%d"),
				Index);
			return false;
		}

		if (SeenAbilityClasses.Contains(LoadedClass))
		{
			PF_LOG(
				TEXT(
					"Initial ability class is duplicated. "
					"AbilityClass=%s"),
				*GetNameSafe(LoadedClass));
			return false;
		}

		if (m_AbilitySystemComponent
			->FindAbilitySpecFromClass(
				AbilityClass) != nullptr)
		{
			PF_LOG(
				TEXT(
					"Initial ability is already granted. "
					"AbilityClass=%s"),
				*GetNameSafe(LoadedClass));
			return false;
		}

		const UObject* AbilityObject = LoadedClass->GetDefaultObject();
		const IPFBattleActionAbilityInterface* BattleActionAbility =
			Cast<IPFBattleActionAbilityInterface>(AbilityObject);

		if (BattleActionAbility)
		{
			const FGameplayTag ActionTag =
				BattleActionAbility->GetActionTag();

			if (!ActionTag.IsValid())
			{
				PF_LOG(
					TEXT(
						"Battle action ability has no valid "
						"ActionTag. AbilityClass=%s"),
					*GetNameSafe(LoadedClass));
				return false;
			}

			if (SeenActionTags.Contains(ActionTag)
				|| m_AbilitySystemComponent
					->HasBattleAction(ActionTag))
			{
				PF_LOG(
					TEXT(
						"Battle action ActionTag is duplicated. "
						"ActionTag=%s"),
					*ActionTag.ToString());
				return false;
			}

			SeenActionTags.Add(ActionTag);
		}

		if (LoadedClass->IsChildOf(UPFDeathAbilityBase::StaticClass()))
		{
			if (bHasDeathAbility
				|| m_AbilitySystemComponent
					->HasDeathAbility())
			{
				PF_LOG(
					TEXT(
						"Initial DeathAbility is duplicated. "
						"AbilityClass=%s"),
					*GetNameSafe(LoadedClass));
				return false;
			}

			bHasDeathAbility = true;
		}

		SeenAbilityClasses.Add(LoadedClass);
	}

	return true;
}

void APFBattleCharacterBase::GrantInitialAbility(
	const TSubclassOf<UPFAbilityBase> AbilityClass)
{
	const FGameplayAbilitySpec AbilitySpec(
		AbilityClass,
		1);

	m_AbilitySystemComponent->GiveAbility(
		AbilitySpec);
}

void APFBattleCharacterBase::HandleHPChanged(
	const FOnAttributeChangeData& ChangeData)
{
	if (ChangeData.NewValue > 0.0f
		|| !IsAlive())
	{
		return;
	}

	if (!m_AbilitySystemComponent
		->AddLooseGameplayTagIfNone(
			SIHGameplayTags::State_Death_Pending))
	{
		return;
	}
}
