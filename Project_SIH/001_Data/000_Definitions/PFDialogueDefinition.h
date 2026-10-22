#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/006_Dialogue/PFDialogueTypes.h"
#include "PFDialogueDefinition.generated.h"

/** 외부 그래프 툴에서 생성한 하나의 대화 실행 데이터를 소유합니다. */
UCLASS()
class PROJECT_SIH_API UPFDialogueDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	static FPrimaryAssetId MakePrimaryAssetID(const FGameplayTag& DialogueID);

	const FGameplayTag& GetDialogueID() const { return m_DialogueID; }
	int32 GetDataVersion() const { return m_DataVersion; }
	FName GetEntryNodeID() const { return m_EntryNodeID; }

	const FPFDialogueRuntimeNode* FindNode(FName NodeID) const;

	/** 실행 전에 모든 노드 참조와 최소 데이터 계약을 검사합니다. */
	bool ValidateRuntimeData(FString* OutError = nullptr) const;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Definition",
		meta = (AllowPrivateAccess = "true",
			DisplayName = "Dialogue ID",
			Categories = "ID.Dialogue"))
	FGameplayTag m_DialogueID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Definition",
		meta = (AllowPrivateAccess = "true",
			DisplayName = "Data Version",
			ClampMin = "1"))
	int32 m_DataVersion = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Definition",
		meta = (AllowPrivateAccess = "true",
			DisplayName = "Entry Node ID"))
	FName m_EntryNodeID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Definition",
		meta = (AllowPrivateAccess = "true",
			DisplayName = "Nodes"))
	TMap<FName, FPFDialogueRuntimeNode> m_Nodes;
};
