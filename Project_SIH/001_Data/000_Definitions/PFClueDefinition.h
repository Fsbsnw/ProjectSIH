#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PFClueDefinition.generated.h"

UCLASS()
class PROJECT_SIH_API UPFClueDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	friend struct FPFClueDefinitionAutomationTestAccessor;
	friend struct FPFMeetingClueDefinitionAutomationTestAccessor;

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	static FPrimaryAssetId MakePrimaryAssetID(const FGameplayTag& ClueID);

	const FGameplayTag& GetClueID() const
	{
		return m_ClueID;
	}

	const FText& GetDisplayName() const
	{
		return m_DisplayName;
	}

	bool IsCombinationResult() const
	{
		return m_bIsCombinationResult;
	}

	bool CanBeUsedAsCombinationSource() const
	{
		return !m_bIsCombinationResult
			&& m_CombinationSourceClueIDs.IsEmpty();
	}

	bool HasValidCombinationRecipe() const;

	bool MatchesCombinationRecipe(
		const FGameplayTagContainer& SourceClueIDs) const;

	const FGameplayTagContainer& GetCombinationSourceClueIDs() const
	{
		return m_CombinationSourceClueIDs;
	}

	bool IsMeetingUsable() const
	{
		return m_bMeetingUsable;
	}

	const FGameplayTag& GetClaimID() const
	{
		return m_ClaimID;
	}

	const FGameplayTag& GetRevealedWeaknessID() const
	{
		return m_RevealedWeaknessID;
	}

private:
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Definition",
		meta = (
			AllowPrivateAccess = "true",
			DisplayName = "Clue ID",
			Categories = "ID.Clue",
			ToolTip = "Clue를 식별하는 고유 GameplayTag입니다. ID.Clue.* 형태입니다"))
	FGameplayTag m_ClueID;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Definition",
		meta = (
			AllowPrivateAccess = "true",
			DisplayName = "Display Name",
			ToolTip = "UI에 표시할 단서의 이름입니다."))
	FText m_DisplayName;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Clue Combination",
		meta = (
			AllowPrivateAccess = "true",
			DisplayName = "Is Combination Result",
			ToolTip = "이 단서가 다른 단서들의 조합 결과인지 여부입니다."))
	bool m_bIsCombinationResult = false;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Clue Combination",
		meta = (
			AllowPrivateAccess = "true",
			DisplayName = "Combination Source Clue IDs",
			Categories = "ID.Clue",
			EditCondition = "m_bIsCombinationResult",
			EditConditionHides,
			ToolTip = "이 단서를 결과로 만드는 순서 없는 입력 단서 집합입니다. 일반 단서는 비우고 조합 결과는 최소 2개를 지정합니다"))
	FGameplayTagContainer m_CombinationSourceClueIDs;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Meeting",
		meta = (
			AllowPrivateAccess = "true",
			DisplayName = "Meeting Usable",
			ToolTip = "이 단서를 Meeting의 제출 후보 목록에 포함할지 여부입니다."))
	bool m_bMeetingUsable = true;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Definition",
		meta = (
			AllowPrivateAccess = "true",
			DisplayName = "Claim ID",
			Categories = "ID.Claim",
			ToolTip = "이 Clue가 반박하는 Claim의 GameplayTag입니다. ID.Claim.* 형태입니다"))
	FGameplayTag m_ClaimID;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Definition",
		meta = (
			AllowPrivateAccess = "true",
			DisplayName = "Revealed Weakness ID",
			Categories = "ID.Weakness",
			ToolTip = "정답 제출 시 공개되는 Weakness의 GameplayTag입니다. ID.Weakness.* 형태입니다"))
	FGameplayTag m_RevealedWeaknessID;
};
