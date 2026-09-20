#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PFCaseDefinition.generated.h"

UCLASS()
class PROJECT_SIH_API UPFCaseDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	static FPrimaryAssetId MakePrimaryAssetID(const FGameplayTag& CaseID);

	const FGameplayTag& GetCaseID() const
	{
		return m_CaseID;
	}

private:
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Definition",
		meta = (AllowPrivateAccess = "true",
			DisplayName = "Case ID",
			Categories = "ID.Case",
			ToolTip = "Case를 식별하는 고유 GameplayTag입니다. ID.Case.*의 형태입니다"))
	FGameplayTag m_CaseID;
};
