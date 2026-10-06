#include "PFBattleSpawner.h"

#include "AbilitySystemComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayEffect.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleMessages.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/001_Data/000_Definitions/PFCaseDefinition.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"
#include "Project_SIH/002_Systems/003_Combat/001_Characters/PFBattleCharacterBase.h"
#include "Project_SIH/002_Systems/003_Combat/001_Characters/PFBoss.h"
#include "Project_SIH/002_Systems/003_Combat/004_Preparation/PFCharacterStatModifierResolver.h"
#include "Project_SIH/002_Systems/003_Combat/004_Preparation/PFCombatProfileResolver.h"
#include "Project_SIH/002_Systems/004_CharacterState/Calculation/PFCharacterStatCalculator.h"
#include "Project_SIH/SIHGameplayTags.h"

void UPFBattleSpawner::Init(
	const FPFBattleSpawnLayout& SpawnLayout)
{
	m_SpawnLayout = SpawnLayout;
}

EPFBattlePreparationResult UPFBattleSpawner::PrepareBattle(
	const FPFBattleEntryContext& EntryContext,
	FPFBattleRuntimeContext& OutRuntimeContext)
{
	OutRuntimeContext = FPFBattleRuntimeContext();

	if (!EntryContext.IsValid())
	{
		PF_LOG(TEXT("Battle preparation context is invalid"));
		return EPFBattlePreparationResult::InvalidContext;
	}

	if (!m_SpawnLayout.IsValid())
	{
		PF_LOG(TEXT("BattleSpawner is not initialized"));
		return EPFBattlePreparationResult::NotReady;
	}

	TArray<FPFPreparedBattleParticipant>
		PreparedParticipants;

	if (!PrepareParticipants(
		EntryContext,
		PreparedParticipants))
	{
		return EPFBattlePreparationResult::NotReady;
	}

	if (!SpawnParticipants(
		PreparedParticipants,
		EntryContext,
		OutRuntimeContext))
	{
		return EPFBattlePreparationResult::SpawnFailed;
	}

	return EPFBattlePreparationResult::Prepared;
}

bool UPFBattleSpawner::PrepareParticipants(
	const FPFBattleEntryContext& EntryContext,
	TArray<FPFPreparedBattleParticipant>&
		OutPreparedParticipants) const
{
	if (!TryPrepareAllyParticipants(
		EntryContext,
		OutPreparedParticipants))
	{
		return false;
	}

	FPFPreparedBattleParticipant PreparedBoss;

	if (!TryPrepareBossParticipant(
		EntryContext.m_CaseID,
		m_SpawnLayout.m_BossTransform,
		PreparedBoss))
	{
		OutPreparedParticipants.Reset();
		return false;
	}

	OutPreparedParticipants.Add(MoveTemp(PreparedBoss));
	return true;
}

bool UPFBattleSpawner::SpawnParticipants(
	const TArray<FPFPreparedBattleParticipant>&
		PreparedParticipants,
	const FPFBattleEntryContext& EntryContext,
	FPFBattleRuntimeContext& OutRuntimeContext) const
{
	const UPFCaseDefinition* CaseDefinition =
		UPFAssetManager::Get().GetCaseDefinition(
			EntryContext.m_CaseID);
	if (!IsValid(CaseDefinition))
	{
		PF_LOG(TEXT("CaseDefinition is unavailable during battle spawn"));
		return false;
	}

	const FPFBossEncounterDefinition& BossEncounter =
		CaseDefinition->GetBossEncounter();
	FPFInitialWeaknessCounts InitialWeaknessCounts;
	TArray<FGameplayTag> VisibleElementIDs;
	if (!TryCalculateInitialWeaknessCounts(
			BossEncounter.m_WeaknessElementIDs,
			EntryContext.m_RevealedWeaknessIDs,
			PreparedParticipants,
			InitialWeaknessCounts,
			VisibleElementIDs))
	{
		return false;
	}

	UClass* WeaknessDebuffClass = nullptr;
	if (InitialWeaknessCounts.m_MatchedSlotCount > 0)
	{
		WeaknessDebuffClass =
			BossEncounter.m_WeaknessDebuffEffect.LoadSynchronous();
		if (!IsValid(WeaknessDebuffClass))
		{
			PF_LOG(TEXT("Boss weakness debuff effect is unavailable"));
			return false;
		}
	}

	TArray<TObjectPtr<APFBattleCharacterBase>>
		SpawnedParticipants;

	SpawnedParticipants.Reserve(
		PreparedParticipants.Num());

	for (int32 Index = 0;
		Index < PreparedParticipants.Num();
		++Index)
	{
		const bool bIsBoss =
			Index == PreparedParticipants.Num() - 1;

		APFBattleCharacterBase* SpawnedParticipant = nullptr;

		if (!TrySpawnParticipant(
			PreparedParticipants[Index],
			bIsBoss
				? EPFBattleSide::Enemy
				: EPFBattleSide::Ally,
			SpawnedParticipant))
		{
			DestroyParticipants(SpawnedParticipants);
			return false;
		}

		SpawnedParticipants.Add(SpawnedParticipant);

		if (bIsBoss)
		{
			APFBoss* Boss = Cast<APFBoss>(SpawnedParticipant);
			if (!IsValid(Boss))
			{
				PF_LOG(TEXT("Spawned enemy is not a Boss"));
				DestroyParticipants(SpawnedParticipants);
				return false;
			}

			if (WeaknessDebuffClass)
			{
				UAbilitySystemComponent* BossASC =
					Boss->GetAbilitySystemComponent();
				const FActiveGameplayEffectHandle AppliedHandle =
					BossASC->ApplyGameplayEffectToSelf(
						WeaknessDebuffClass->GetDefaultObject<UGameplayEffect>(),
						static_cast<float>(InitialWeaknessCounts.m_MatchedSlotCount),
						BossASC->MakeEffectContext());
				if (!AppliedHandle.WasSuccessfullyApplied())
				{
					PF_LOG(TEXT("Failed to apply Boss weakness debuff"));
					DestroyParticipants(SpawnedParticipants);
					return false;
				}
			}
		}
	}

	OutRuntimeContext.m_CaseID = EntryContext.m_CaseID;
	OutRuntimeContext.m_InitialWeaknessCounts = InitialWeaknessCounts;
	OutRuntimeContext.m_Participants =
		MoveTemp(SpawnedParticipants);

	TArray<FGameplayTag> CharacterIDs;
	EntryContext.m_Party.m_CharacterIDs.GetGameplayTagArray(CharacterIDs);

	FPFBattleParticipantsInitializedMessage ParticipantsMessage;
	ParticipantsMessage.m_Allies.Reserve(CharacterIDs.Num());
	for (int32 Index = 0; Index < CharacterIDs.Num(); ++Index)
	{
		FPFBattleParticipantEntry& Entry =
			ParticipantsMessage.m_Allies.AddDefaulted_GetRef();
		Entry.m_Character = OutRuntimeContext.m_Participants[Index];
		Entry.m_CharacterID = CharacterIDs[Index];
	}
	ParticipantsMessage.m_Boss.m_Character =
		OutRuntimeContext.m_Participants.Last();
	ParticipantsMessage.m_Boss.m_CharacterID =
		BossEncounter.m_CharacterID;

	UGameplayMessageSubsystem& MessageSubsystem =
		UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(
		SIHGameplayTags::Message_Battle_Participants_Initialized.GetTag(),
		ParticipantsMessage);

	FPFBattleWeaknessSlotsMessage WeaknessMessage;
	WeaknessMessage.m_VisibleElementIDs = MoveTemp(VisibleElementIDs);
	MessageSubsystem.BroadcastMessage(
		SIHGameplayTags::Message_Battle_WeaknessSlots_Initialized.GetTag(),
		WeaknessMessage);

	return true;
}

bool UPFBattleSpawner::TryPrepareAllyParticipants(
	const FPFBattleEntryContext& Context,
	TArray<FPFPreparedBattleParticipant>&
		OutPreparedParticipants) const
{
	OutPreparedParticipants.Reset();

	UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		PF_LOG(TEXT("BattleSpawner World is unavailable"));
		return false;
	}

	UGameInstance* GameInstance =
		World->GetGameInstance();

	if (!IsValid(GameInstance))
	{
		PF_LOG(TEXT("GameInstance is unavailable"));
		return false;
	}

	FPFCombatProfileResolver CombatProfileResolver;

	FPFCharacterStatModifierResolver
		StatModifierResolver(*GameInstance);

	TArray<FGameplayTag> CharacterIDs;

	Context.m_Party.m_CharacterIDs.GetGameplayTagArray(
		CharacterIDs);

	OutPreparedParticipants.Reserve(
		CharacterIDs.Num());

	for (int32 Index = 0;
		Index < CharacterIDs.Num();
		++Index)
	{
		const FGameplayTag& CharacterID =
			CharacterIDs[Index];

		FPFResolvedCombatProfile ResolvedProfile;

		if (!CombatProfileResolver.ResolveAllyProfile(
			CharacterID,
			ResolvedProfile))
		{
			PF_LOG(
				TEXT(
					"Failed to resolve ally CombatProfile. "
					"CharacterID=%s"),
				*CharacterID.ToString());

			OutPreparedParticipants.Reset();
			return false;
		}

		if (!PFGameplayTagUtilities::IsValidChildTag(
				ResolvedProfile.m_ElementID,
				SIHGameplayTags::ID_Element))
		{
			PF_LOG(TEXT("Ally ElementID is invalid. CharacterID=%s"),
				*CharacterID.ToString());
			OutPreparedParticipants.Reset();
			return false;
		}

		FPFResolvedCharacterStatModifiers
			ResolvedModifiers;

		if (!StatModifierResolver.Resolve(
			CharacterID,
			ResolvedModifiers))
		{
			PF_LOG(
				TEXT(
					"Failed to resolve character "
					"Stat modifiers. CharacterID=%s"),
				*CharacterID.ToString());

			OutPreparedParticipants.Reset();
			return false;
		}

		FPFPreparedBattleParticipant
			PreparedParticipant;

		if (!FPFCharacterStatCalculator::TryCalculate(
			ResolvedProfile.m_BaseStats,
			ResolvedProfile.m_LevelBonusTable,
			ResolvedModifiers.m_CharacterLevel,
			ResolvedModifiers.m_EquipmentModifiers,
			PreparedParticipant.m_InitialStats))
		{
			PF_LOG(
				TEXT(
					"Failed to calculate ally stats. "
					"CharacterID=%s, CharacterLevel=%d"),
				*CharacterID.ToString(),
				ResolvedModifiers.m_CharacterLevel);

			OutPreparedParticipants.Reset();
			return false;
		}

		PreparedParticipant.m_CharacterClassToSpawn =
			ResolvedProfile.m_CharacterClassToSpawn;
		PreparedParticipant.m_ElementID =
			ResolvedProfile.m_ElementID;

		PreparedParticipant.m_InitialAbilities =
			MoveTemp(
				ResolvedProfile.m_InitialAbilities);

		PreparedParticipant.m_SpawnTransform =
			m_SpawnLayout.m_AllyTransforms[Index];

		OutPreparedParticipants.Add(
			MoveTemp(PreparedParticipant));
	}

	return true;
}

bool UPFBattleSpawner::TryPrepareBossParticipant(
	const FGameplayTag& CaseID,
	const FTransform& SpawnTransform,
	FPFPreparedBattleParticipant&
		OutPreparedParticipant) const
{
	OutPreparedParticipant =
		FPFPreparedBattleParticipant();

	FPFCombatProfileResolver CombatProfileResolver;
	FPFResolvedCombatProfile ResolvedProfile;

	if (!CombatProfileResolver.ResolveBossProfile(
		CaseID,
		ResolvedProfile))
	{
		PF_LOG(
			TEXT(
				"Failed to resolve Boss CombatProfile. "
				"CaseID=%s"),
			*CaseID.ToString());
		return false;
	}

	if (!FPFCharacterStatCalculator::TryCalculate(
		ResolvedProfile.m_BaseStats,
		OutPreparedParticipant.m_InitialStats))
	{
		PF_LOG(
			TEXT(
				"Failed to calculate Boss BaseStats. "
				"CaseID=%s"),
			*CaseID.ToString());
		return false;
	}

	OutPreparedParticipant.m_CharacterClassToSpawn =
		ResolvedProfile.m_CharacterClassToSpawn;

	OutPreparedParticipant.m_InitialAbilities =
		MoveTemp(ResolvedProfile.m_InitialAbilities);

	OutPreparedParticipant.m_SpawnTransform =
		SpawnTransform;

	return true;
}

bool UPFBattleSpawner::TrySpawnParticipant(
	const FPFPreparedBattleParticipant& PreparedParticipant,
	const EPFBattleSide ExpectedSide,
	APFBattleCharacterBase*& OutParticipant) const
{
	OutParticipant = nullptr;

	UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		PF_LOG(TEXT("BattleSpawner World is unavailable"));
		return false;
	}

	APFBattleCharacterBase* SpawnedParticipant =
		World->SpawnActor<APFBattleCharacterBase>(
			PreparedParticipant.m_CharacterClassToSpawn,
			PreparedParticipant.m_SpawnTransform);

	if (!IsValid(SpawnedParticipant))
	{
		PF_LOG(
			TEXT(
				"Failed to spawn battle participant. "
				"Class=%s"),
			*GetNameSafe(
				PreparedParticipant
					.m_CharacterClassToSpawn.Get()));
		return false;
	}

	if (SpawnedParticipant->GetBattleSide()
		!= ExpectedSide)
	{
		PF_LOG(
			TEXT(
				"Spawned participant has unexpected "
				"BattleSide. Actor=%s, Expected=%d, "
				"Actual=%d"),
			*GetNameSafe(SpawnedParticipant),
			static_cast<uint8>(ExpectedSide),
			static_cast<uint8>(
				SpawnedParticipant->GetBattleSide()));

		SpawnedParticipant->Destroy();
		return false;
	}

	if (!SpawnedParticipant->TryInitializeBattleAttributes(
		PreparedParticipant.m_InitialStats))
	{
		PF_LOG(
			TEXT(
				"Failed to initialize battle attributes. "
				"Actor=%s"),
			*GetNameSafe(SpawnedParticipant));

		SpawnedParticipant->Destroy();
		return false;
	}

	if (!SpawnedParticipant->TryGrantInitialAbilities(
		PreparedParticipant.m_InitialAbilities))
	{
		PF_LOG(
			TEXT(
				"Failed to grant initial abilities. "
				"Actor=%s"),
			*GetNameSafe(SpawnedParticipant));

		SpawnedParticipant->Destroy();
		return false;
	}

	OutParticipant = SpawnedParticipant;
	return true;
}

bool UPFBattleSpawner::TryCalculateInitialWeaknessCounts(
	const TArray<FGameplayTag>& WeaknessElementIDs,
	const TArray<FGameplayTag>& RevealedWeaknessIDs,
	const TArray<FPFPreparedBattleParticipant>& PreparedParticipants,
	FPFInitialWeaknessCounts& OutCounts,
	TArray<FGameplayTag>& OutVisibleElementIDs) const
{
	OutCounts = FPFInitialWeaknessCounts();
	OutVisibleElementIDs.Reset();
	if (WeaknessElementIDs.Num()
		!= FPFInitialWeaknessCounts::SlotCount)
	{
		PF_LOG(TEXT("Boss must have exactly four weakness elements"));
		return false;
	}

	for (const FGameplayTag& WeaknessElementID : WeaknessElementIDs)
	{
		if (!PFGameplayTagUtilities::IsValidChildTag(
				WeaknessElementID,
				SIHGameplayTags::ID_Element))
		{
			PF_LOG(TEXT("Boss weakness ElementID is invalid"));
			return false;
		}
	}

	TArray<bool> RevealedSlots;
	RevealedSlots.Init(false, WeaknessElementIDs.Num());
	// 회의에서 같은 속성을 여러 번 밝혀도 원본 순서의 서로 다른 슬롯에 대응시킨다.
	for (const FGameplayTag& RevealedWeaknessID : RevealedWeaknessIDs)
	{
		bool bFoundSlot = false;
		for (int32 SlotIndex = 0;
			SlotIndex < WeaknessElementIDs.Num();
			++SlotIndex)
		{
			if (WeaknessElementIDs[SlotIndex] == RevealedWeaknessID
				&& !RevealedSlots[SlotIndex])
			{
				RevealedSlots[SlotIndex] = true;
				++OutCounts.m_RevealedSlotCount;
				bFoundSlot = true;
				break;
			}
		}

		if (!bFoundSlot)
		{
			PF_LOG(TEXT("Revealed weakness has no remaining Boss slot. ElementID=%s"),
				*RevealedWeaknessID.ToString());
			return false;
		}
	}

	TArray<bool> MatchedSlots;
	MatchedSlots.Init(false, WeaknessElementIDs.Num());
	// 파티 매칭은 회의 공개와 독립적으로 세며, 한 파티원은 한 슬롯만 매칭한다.
	for (int32 ParticipantIndex = 0;
		ParticipantIndex < PreparedParticipants.Num() - 1;
		++ParticipantIndex)
	{
		const FGameplayTag& AllyElementID =
			PreparedParticipants[ParticipantIndex].m_ElementID;
		for (int32 SlotIndex = 0;
			SlotIndex < WeaknessElementIDs.Num();
			++SlotIndex)
		{
			if (WeaknessElementIDs[SlotIndex] == AllyElementID
				&& !MatchedSlots[SlotIndex])
			{
				MatchedSlots[SlotIndex] = true;
				++OutCounts.m_MatchedSlotCount;
				break;
			}
		}
	}

	OutVisibleElementIDs.Init(
		FGameplayTag(),
		WeaknessElementIDs.Num());

	for (int32 SlotIndex = 0;
		SlotIndex < WeaknessElementIDs.Num();
		++SlotIndex)
	{
		if (RevealedSlots[SlotIndex] || MatchedSlots[SlotIndex])
		{
			OutVisibleElementIDs[SlotIndex] =
				WeaknessElementIDs[SlotIndex];
		}
	}

	return true;
}

void UPFBattleSpawner::DestroyParticipants(
	TArray<TObjectPtr<APFBattleCharacterBase>>&
		Participants) const
{
	for (APFBattleCharacterBase* Participant
		: Participants)
	{
		if (IsValid(Participant))
		{
			Participant->Destroy();
		}
	}

	Participants.Reset();
}
