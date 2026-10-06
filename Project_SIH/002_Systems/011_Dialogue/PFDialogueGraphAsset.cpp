#include "PFDialogueGraphAsset.h"

const FPFDialogueRuntimeNode* UPFDialogueGraphAsset::FindRuntimeNode(
	const FGuid& NodeID) const
{
	return m_RuntimeNodes.FindByPredicate(
		[&NodeID](const FPFDialogueRuntimeNode& Node)
		{
			return Node.NodeID == NodeID;
		});
}

#if WITH_EDITOR

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "PFDialogueRuntimeNodeProvider.h"
#include "UObject/ObjectSaveContext.h"

void UPFDialogueGraphAsset::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

#if WITH_EDITORONLY_DATA
	if (m_EditorGraph)
	{
		for (UEdGraphNode* Node : m_EditorGraph->Nodes)
		{
			if (Node)
			{
				Node->ReconstructNode();
			}
		}

		m_EditorGraph->NotifyGraphChanged();
	}
#endif
}

void UPFDialogueGraphAsset::PreSave(FObjectPreSaveContext SaveContext)
{
	RebuildRuntimeGraph();
	Super::PreSave(SaveContext);
}

void UPFDialogueGraphAsset::RebuildRuntimeGraph()
{
	m_RuntimeNodes.Reset();
	m_StartNodeID.Invalidate();

#if WITH_EDITORONLY_DATA
	if (!m_EditorGraph)
	{
		return;
	}

	for (const UEdGraphNode* EditorNode : m_EditorGraph->Nodes)
	{
		const IPFDialogueRuntimeNodeProvider* Provider =
			Cast<IPFDialogueRuntimeNodeProvider>(EditorNode);
		if (!Provider)
		{
			continue;
		}

		FPFDialogueRuntimeNode RuntimeNode;
		if (!Provider->BuildRuntimeNode(RuntimeNode)
			|| !RuntimeNode.NodeID.IsValid())
		{
			continue;
		}

		if (RuntimeNode.NodeType == EPFDialogueGraphNodeType::Root)
		{
			m_StartNodeID = RuntimeNode.NodeID;
		}

		m_RuntimeNodes.Add(MoveTemp(RuntimeNode));
	}
#endif
}

#endif
