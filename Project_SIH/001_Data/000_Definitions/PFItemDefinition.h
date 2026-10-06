#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/005_Item/PFItemTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "PFItemDefinition.generated.h"

UCLASS()
class PROJECT_SIH_API UPFItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	static FPrimaryAssetId MakePrimaryAssetID(
		const FGameplayTag& ItemDefinitionID);

	template <typename FragmentType>
	const FragmentType* FindFragment() const
	{
		static_assert(
			TIsDerivedFrom<FragmentType, FPFItemFragment>::IsDerived,
			"FragmentType must derive from FPFItemFragment.");

		for (const TInstancedStruct<FPFItemFragment>& Fragment : m_Fragments)
		{
			if (const FragmentType* FoundFragment =
				Fragment.GetPtr<FragmentType>())
			{
				return FoundFragment;
			}
		}

		return nullptr;
	}

private:
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Definition",
		meta = (
			AllowPrivateAccess = "true",
			DisplayName = "Item Definition ID",
			Categories = "ID.Item",
			ToolTip = "아이템 Definition을 식별하는 GameplayTag입니다. ID.Item.* 형태입니다"))
	FGameplayTag m_DefinitionTag;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Definition|Fragments",
		meta = (
			DisplayName = "Fragments",
			ExcludeBaseStruct,
			ToolTip = "아이템의 기능과 분류를 구성하는 Fragment 목록입니다"))
	TArray<TInstancedStruct<FPFItemFragment>> m_Fragments;
};
