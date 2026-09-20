#include "PFClueSystem.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/001_Data/000_Definitions/PFClueDefinition.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"
#include "Project_SIH/SIHGameplayTags.h"

void UPFClueSystem::Init()
{
	m_ActiveCaseID = FGameplayTag();
	m_AcquiredClueIDs.Reset();
	m_CombinationAttempts.Reset();
}

void UPFClueSystem::Deinit()
{
	m_ActiveCaseID = FGameplayTag();
	m_AcquiredClueIDs.Reset();
	m_CombinationAttempts.Reset();
}

bool UPFClueSystem::TryResetForCase(const FGameplayTag& CaseID)
{
	if (!PFGameplayTagUtilities::IsValidChildTag(
		CaseID,
		SIHGameplayTags::ID_Case))
	{
		PF_LOG(
			TEXT("CaseID is outside the Case ID domain. CaseID=%s"),
			*CaseID.ToString());
		return false;
	}

	if (!UPFAssetManager::Get().GetCaseDefinition(CaseID))
	{
		PF_LOG(
			TEXT("Case definition is unavailable. CaseID=%s"),
			*CaseID.ToString());
		return false;
	}

	m_ActiveCaseID = CaseID;
	m_AcquiredClueIDs.Reset();
	m_CombinationAttempts.Reset();
	return true;
}

EPFClueAcquireResult UPFClueSystem::TryAcquireClue(const FGameplayTag& ClueID)
{
	if (!m_ActiveCaseID.IsValid())
	{
		PF_LOG(TEXT("There is no active Case for clue acquisition."));
		return EPFClueAcquireResult::InactiveCase;
	}

	if (!PFGameplayTagUtilities::IsValidChildTag(
		ClueID,
		SIHGameplayTags::ID_Clue))
	{
		PF_LOG(
			TEXT("ClueID is outside the Clue ID domain. ClueID=%s"),
			*ClueID.ToString());
		return EPFClueAcquireResult::InvalidClueID;
	}

	const UPFClueDefinition* ClueDefinition =
		UPFAssetManager::Get().GetClueDefinition(ClueID);

	if (!ClueDefinition)
	{
		PF_LOG(
			TEXT("Clue definition is unavailable. ClueID=%s"),
			*ClueID.ToString());
		return EPFClueAcquireResult::DefinitionUnavailable;
	}

	if (m_AcquiredClueIDs.HasTagExact(ClueID))
	{
		return EPFClueAcquireResult::AlreadyOwned;
	}

	m_AcquiredClueIDs.AddTag(ClueID);
	return EPFClueAcquireResult::Acquired;
}

FPFClueCombinationResult UPFClueSystem::TryCombineClues(const FGameplayTagContainer& SourceClueIDs)
{
	FPFClueCombinationResult Result;

	// 활성 Case가 없는 경우 조합을 진행할 수 없음
	if (!m_ActiveCaseID.IsValid())
	{
		PF_LOG(TEXT("There is no active Case for Clue combination."));
		Result.m_Result = EPFClueCombineResult::InactiveCase;
		return Result;
	}

	// 조합에 필요한 최소 단서 개수를 만족하지 못한 경우
	if (SourceClueIDs.Num() < 2)
	{
		Result.m_Result = EPFClueCombineResult::InvalidSourceClueCount;
		return Result;
	}

	// 조합에 사용되는 모든 단서의 유효성과 사용 가능 여부를 검증
	for (const FGameplayTag& SourceClueID : SourceClueIDs)
	{
		// 유효한 Clue ID가 아닌 경우
		if (!PFGameplayTagUtilities::IsValidChildTag(
				SourceClueID,
				SIHGameplayTags::ID_Clue))
		{
			PF_LOG(
				TEXT(
					"Clue combination contains an invalid SourceClueID. "
					"SourceClueID=%s"),
				*SourceClueID.ToString());
			Result.m_Result = EPFClueCombineResult::InvalidSourceClueID;
			return Result;
		}

		// 아직 획득하지 않은 단서인 경우
		if (!m_AcquiredClueIDs.HasTagExact(SourceClueID))
		{
			Result.m_Result = EPFClueCombineResult::SourceClueNotOwned;
			return Result;
		}

		const UPFClueDefinition* ClueDefinition =
			UPFAssetManager::Get().GetClueDefinition(SourceClueID);
		// 단서 정의를 찾을 수 없는 경우
		if (!ClueDefinition)
		{
			Result.m_Result = EPFClueCombineResult::SourceDefinitionUnavailable;
			return Result;
		}

		// 조합 재료로 사용할 수 없는 단서인 경우
		if (!ClueDefinition->CanBeUsedAsCombinationSource())
		{
			Result.m_Result = EPFClueCombineResult::SourceClueNotCombinable;
			return Result;
		}
	}

	const UPFClueDefinition* ResultClueDefinition =
		UPFAssetManager::Get().GetClueCombinationDefinition(SourceClueIDs);
	if (!ResultClueDefinition)
	{
		const int32 ExistingAttemptIndex = FindCombinationAttemptIndex(SourceClueIDs);
		// 현재 조합 정의는 없지만 이전 시도 기록이 존재하는 경우
		if (ExistingAttemptIndex != INDEX_NONE)
		{
			// 이전에도 실패한 조합이면 이미 검토한 조합으로 처리
			const FPFClueCombinationAttempt& ExistingAttempt = m_CombinationAttempts[ExistingAttemptIndex];
			if (ExistingAttempt.m_Result == EPFClueCombinationAttemptResult::Failed)
			{
				Result.m_Result = EPFClueCombineResult::AlreadyReviewed;
				return Result;
			}

			// 성공 기록은 남아 있지만 현재 조합 정의를 찾을 수 없는 비정상 상태
			PF_LOG(
				TEXT(
					"A successful Clue combination record no longer has "
					"a matching result definition. ResultClueID=%s"),
				*ExistingAttempt.m_ResultClueID.ToString());
			Result.m_ResultClueID = ExistingAttempt.m_ResultClueID;
			Result.m_Result = EPFClueCombineResult::InvalidResultDefinition;
			return Result;
		}

		// 처음 시도한 조합이면서 대응하는 조합 정의가 없는 경우 실패로 기록
		FPFClueCombinationAttempt& Attempt = m_CombinationAttempts.AddDefaulted_GetRef();
		Attempt.m_SourceClueIDs = SourceClueIDs;
		Attempt.m_Result = EPFClueCombinationAttemptResult::Failed;
		Result.m_Result = EPFClueCombineResult::RecipeUnavailable;
		return Result;
	}

	// 조합 결과 단서 정의의 조합 정보가 유효하지 않은 경우
	Result.m_ResultClueID = ResultClueDefinition->GetClueID();
	if (!ResultClueDefinition->HasValidCombinationRecipe())
	{
		PF_LOG(
			TEXT("Combination result Clue definition is unavailable or invalid. ClueID=%s"),
			*Result.m_ResultClueID.ToString());
		Result.m_Result = EPFClueCombineResult::InvalidResultDefinition;
		return Result;
	}

	// 조합 결과 단서를 이미 획득한 경우
	if (m_AcquiredClueIDs.HasTagExact(Result.m_ResultClueID))
	{
		Result.m_Result = EPFClueCombineResult::ResultAlreadyOwned;
		return Result;
	}

	// 조합 가능한 단서인 경우, 획득한 단서 목록에 추가
	m_AcquiredClueIDs.AddTag(Result.m_ResultClueID);

	const int32 ExistingAttemptIndex = FindCombinationAttemptIndex(SourceClueIDs);
	
	// 기존 시도 기록이 있으면 갱신하고, 없으면 새 성공 기록을 생성
	FPFClueCombinationAttempt* Attempt = ExistingAttemptIndex == INDEX_NONE
		? &m_CombinationAttempts.AddDefaulted_GetRef()
		: &m_CombinationAttempts[ExistingAttemptIndex];
	
	// 현재 조합의 성공 결과를 기록
	Attempt->m_SourceClueIDs = SourceClueIDs;
	Attempt->m_Result = EPFClueCombinationAttemptResult::Success;
	Attempt->m_ResultClueID = Result.m_ResultClueID;

	Result.m_Result = EPFClueCombineResult::Combined;
	return Result;
}

int32 UPFClueSystem::FindCombinationAttemptIndex(const FGameplayTagContainer& SourceClueIDs) const
{
	return m_CombinationAttempts.IndexOfByPredicate(
		[&SourceClueIDs](const FPFClueCombinationAttempt& Attempt)
		{
			return Attempt.MatchesSourceClueIDs(SourceClueIDs);
		});
}

bool UPFClueSystem::HasClue(const FGameplayTag& ClueID) const
{
	return PFGameplayTagUtilities::IsValidChildTag(
			ClueID,
			SIHGameplayTags::ID_Clue)
		&& m_AcquiredClueIDs.HasTagExact(ClueID);
}

bool UPFClueSystem::TryGetAcquiredClueIDs(FGameplayTagContainer& OutClueIDs) const
{
	OutClueIDs.Reset();

	if (!m_ActiveCaseID.IsValid())
	{
		return false;
	}

	OutClueIDs = m_AcquiredClueIDs;
	return true;
}

bool UPFClueSystem::TryGetCombinationAttempts(TArray<FPFClueCombinationAttempt>& OutAttempts) const
{
	OutAttempts.Reset();

	if (!m_ActiveCaseID.IsValid())
	{
		return false;
	}

	OutAttempts = m_CombinationAttempts;
	return true;
}
