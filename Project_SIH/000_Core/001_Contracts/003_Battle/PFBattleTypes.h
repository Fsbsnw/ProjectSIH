#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/000_Core/001_Contracts/002_Party/PFPartyTypes.h"
#include "PFBattleTypes.generated.h"

class APFBattleCharacterBase;
class AActor;

enum class EPFBattleSide : uint8
{
	None,
	Ally,
	Enemy,
};

UENUM(BlueprintType)
enum class EPFTargetRelation : uint8
{
	Self,
	SameSide,
	OpposingSide,
};

UENUM(BlueprintType)
enum class EPFTargetCount : uint8
{
	Single,
	All,
};

enum class EPFBattleResult : uint8
{
	None,
	NormalVictory,
	ForcedEviction,
	Defeat,
};

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFBattleTargetRule
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		meta = (DisplayName = "Target Relation"))
	EPFTargetRelation m_Relation = EPFTargetRelation::OpposingSide;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		meta = (DisplayName = "Target Count"))
	EPFTargetCount m_Count = EPFTargetCount::Single;

	bool IsValid() const
	{
		return m_Relation != EPFTargetRelation::Self
			|| m_Count == EPFTargetCount::Single;
	}
};

USTRUCT()
struct PROJECT_SIH_API FPFBattleTargetSelection
{
	GENERATED_BODY()

	UPROPERTY()
	EPFTargetCount m_TargetCount = EPFTargetCount::Single;

	UPROPERTY()
	TArray<TObjectPtr<APFBattleCharacterBase>> m_ValidTargets;

	UPROPERTY()
	TObjectPtr<APFBattleCharacterBase> m_DefaultTarget;
};

struct PROJECT_SIH_API FPFBattleActionRequest
{
	AActor* m_Requester = nullptr;
	FGameplayTag m_ActionTag;
	AActor* m_Target = nullptr;

	bool IsValid() const
	{
		return m_Requester != nullptr
			&& m_ActionTag.IsValid();
	}

	bool IsTargetSelectionValid(
		EPFTargetRelation Relation,
		EPFTargetCount Count) const;
};

struct PROJECT_SIH_API FPFBattleSpawnLayout
{
	TArray<FTransform> m_AllyTransforms;
	FTransform m_BossTransform;

	bool IsValid() const
	{
		if (m_AllyTransforms.Num()
			!= FPFPartyData::RequiredMemberCount)
		{
			return false;
		}

		for (const FTransform& AllyTransform
			: m_AllyTransforms)
		{
			if (!AllyTransform.IsValid())
			{
				return false;
			}
		}

		return m_BossTransform.IsValid();
	}
};

// 준비 단계의 공개·파티 매칭 개수만 전달한다. 같은 속성의 중복 슬롯도 각각 센다.
struct FPFInitialWeaknessCounts
{
	static constexpr int32 SlotCount = 4;

	int32 m_RevealedSlotCount = 0;
	int32 m_MatchedSlotCount = 0;

	bool IsValid() const
	{
		return m_RevealedSlotCount >= 0
			&& m_RevealedSlotCount <= SlotCount
			&& m_MatchedSlotCount >= 0
			&& m_MatchedSlotCount <= SlotCount;
	}

	EPFBattleResult DetermineResultOnVictory() const
	{
		return m_RevealedSlotCount == SlotCount
			&& m_MatchedSlotCount == SlotCount
			? EPFBattleResult::ForcedEviction
			: EPFBattleResult::NormalVictory;
	}
};

struct PROJECT_SIH_API FPFBattleRuntimeContext
{
	FGameplayTag m_CaseID;

	TArray<TObjectPtr<APFBattleCharacterBase>>
		m_Participants;

	FPFInitialWeaknessCounts m_InitialWeaknessCounts;

	bool IsValid() const
	{
		if (!m_CaseID.IsValid()
			|| m_Participants.IsEmpty()
			|| !m_InitialWeaknessCounts.IsValid())
		{
			return false;
		}

		for (const TObjectPtr<APFBattleCharacterBase>&
			Participant : m_Participants)
		{
			if (!Participant)
			{
				return false;
			}
		}

		return true;
	}
};
