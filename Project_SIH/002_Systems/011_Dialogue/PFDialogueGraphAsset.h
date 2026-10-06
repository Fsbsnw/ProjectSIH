#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PFDialogueGraphTypes.h"
#include "PFDialogueGraphAsset.generated.h"

class UEdGraph;
class UDataTable;

/**
 * Persistent editor graph plus its compiled runtime representation.
 */
UCLASS()
class PROJECT_SIH_API UPFDialogueGraphAsset final : public UDataAsset
{
	GENERATED_BODY()

public:
	const TArray<FPFDialogueRuntimeNode>& GetRuntimeNodes() const
	{
		return m_RuntimeNodes;
	}

	const FPFDialogueRuntimeNode* FindRuntimeNode(const FGuid& NodeID) const;

	const FGuid& GetStartNodeID() const
	{
		return m_StartNodeID;
	}

	UDataTable* GetDialogueDataTable() const
	{
		return m_DialogueDataTable;
	}

	void SetDialogueDataTable(UDataTable* InDialogueDataTable)
	{
		m_DialogueDataTable = InDialogueDataTable;
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	void RebuildRuntimeGraph();
#endif

#if WITH_EDITORONLY_DATA
	UEdGraph* GetEditorGraph() const
	{
		return m_EditorGraph;
	}

	void SetEditorGraph(UEdGraph* InEditorGraph)
	{
		m_EditorGraph = InEditorGraph;
	}
#endif

private:
	/** Runtime graph generated from editor nodes and pin links. */
	UPROPERTY()
	TArray<FPFDialogueRuntimeNode> m_RuntimeNodes;

	UPROPERTY()
	FGuid m_StartNodeID;

	/** Single source of dialogue and choice text available to graph nodes. */
	UPROPERTY(
		EditAnywhere,
		Category = "Dialogue",
		meta = (RequiredAssetDataTags = "RowStructure=/Script/Project_SIH.PFDialogueTableRow"))
	TObjectPtr<UDataTable> m_DialogueDataTable;

#if WITH_EDITORONLY_DATA
	/** Owns node positions, node data, pins, and pin links for the prototype. */
	UPROPERTY()
	TObjectPtr<UEdGraph> m_EditorGraph;
#endif
};
