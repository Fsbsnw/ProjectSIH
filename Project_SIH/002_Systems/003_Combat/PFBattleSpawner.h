#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/000_Core/001_Contracts/000_Flow/PFGameFlowTypes.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleTypes.h"
#include "Project_SIH/000_Core/001_Contracts/004_Character/PFCharacterStatTypes.h"
#include "UObject/Object.h"
#include "PFBattleSpawner.generated.h"

class APFBattleCharacterBase;
class UPFAbilityBase;

struct FPFPreparedBattleParticipant
{
	TSubclassOf<APFBattleCharacterBase>
		m_CharacterClassToSpawn;

	FPFCharacterStatValues m_InitialStats;
	FGameplayTag m_ElementID;

	TArray<TSubclassOf<UPFAbilityBase>>
		m_InitialAbilities;

	FTransform m_SpawnTransform;
};

enum class EPFBattlePreparationResult : uint8
{
	None,
	Prepared,
	InvalidContext,
	NotReady,
	SpawnFailed,
};

UCLASS()
class PROJECT_SIH_API UPFBattleSpawner : public UObject
{
	GENERATED_BODY()

public:
	void Init(
		const FPFBattleSpawnLayout& SpawnLayout);

	EPFBattlePreparationResult PrepareBattle(
		const FPFBattleEntryContext& EntryContext,
		FPFBattleRuntimeContext& OutRuntimeContext);

private:
	bool PrepareParticipants(
		const FPFBattleEntryContext& EntryContext,
		TArray<FPFPreparedBattleParticipant>&
			OutPreparedParticipants) const;

	bool SpawnParticipants(
		const TArray<FPFPreparedBattleParticipant>&
			PreparedParticipants,
		const FPFBattleEntryContext& EntryContext,
		FPFBattleRuntimeContext& OutRuntimeContext) const;

	bool TryPrepareAllyParticipants(
		const FPFBattleEntryContext& Context,
		TArray<FPFPreparedBattleParticipant>&
			OutPreparedParticipants) const;

	bool TryPrepareBossParticipant(
		const FGameplayTag& CaseID,
		const FTransform& SpawnTransform,
		FPFPreparedBattleParticipant&
			OutPreparedParticipant) const;

	bool TrySpawnParticipant(
		const FPFPreparedBattleParticipant&
			PreparedParticipant,
		EPFBattleSide ExpectedSide,
		APFBattleCharacterBase*& OutParticipant) const;

	bool TryCalculateInitialWeaknessCounts(
		const TArray<FGameplayTag>& WeaknessElementIDs,
		const TArray<FGameplayTag>& RevealedWeaknessIDs,
		const TArray<FPFPreparedBattleParticipant>& PreparedParticipants,
		FPFInitialWeaknessCounts& OutCounts,
		TArray<FGameplayTag>& OutVisibleElementIDs) const;

	// TODO: Remove partial spawn rollback after automatic
	// battle-world restart recovery is implemented.
	void DestroyParticipants(
		TArray<TObjectPtr<APFBattleCharacterBase>>&
			Participants) const;

	FPFBattleSpawnLayout m_SpawnLayout;
};
