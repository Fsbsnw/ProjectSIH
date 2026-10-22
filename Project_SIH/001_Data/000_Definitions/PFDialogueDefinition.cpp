#include "PFDialogueDefinition.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/SIHGameplayTags.h"

FPrimaryAssetId UPFDialogueDefinition::GetPrimaryAssetId() const
{
	return MakePrimaryAssetID(m_DialogueID);
}

FPrimaryAssetId UPFDialogueDefinition::MakePrimaryAssetID(
	const FGameplayTag& DialogueID)
{
	if (!DialogueID.IsValid())
	{
		PF_LOG(
			TEXT("DialogueID is invalid. Value=%s"),
			*DialogueID.ToString());
		return FPrimaryAssetId();
	}

	if (!PFGameplayTagUtilities::IsValidChildTag(
		DialogueID,
		SIHGameplayTags::ID_Dialogue))
	{
		PF_LOG(
			TEXT(
				"Tag is outside the Dialogue ID domain. "
				"DialogueID=%s, RequiredRoot=%s"),
			*DialogueID.ToString(),
			*SIHGameplayTags::ID_Dialogue.GetTag().ToString());
		return FPrimaryAssetId();
	}

	const FPrimaryAssetType AssetType(TEXT("Dialogue"));
	return FPrimaryAssetId(AssetType, DialogueID.GetTagName());
}

const FPFDialogueRuntimeNode* UPFDialogueDefinition::FindNode(
	const FName NodeID) const
{
	return m_Nodes.Find(NodeID);
}

bool UPFDialogueDefinition::ValidateRuntimeData(
	FString* OutError) const
{
	auto Fail = [OutError](const FString& Message)
	{
		if (OutError)
		{
			*OutError = Message;
		}
		return false;
	};

	if (!MakePrimaryAssetID(m_DialogueID).IsValid())
	{
		return Fail(TEXT("Dialogue ID is invalid."));
	}

	if (m_DataVersion < 1)
	{
		return Fail(TEXT("Data Version must be at least 1."));
	}

	if (m_EntryNodeID.IsNone()
		|| !m_Nodes.Contains(m_EntryNodeID))
	{
		return Fail(TEXT("Entry Node ID does not reference an existing node."));
	}

	TSet<FName> LineIDs;
	for (const TPair<FName, FPFDialogueRuntimeNode>& Pair : m_Nodes)
	{
		const FName NodeID = Pair.Key;
		const FPFDialogueRuntimeNode& Node = Pair.Value;

		if (NodeID.IsNone() || !Node.IsStructurallyValid())
		{
			return Fail(FString::Printf(
				TEXT("Node is structurally invalid. NodeID=%s"),
				*NodeID.ToString()));
		}

		for (const FPFDialogueLine& Line : Node.m_Lines)
		{
			if (!PFGameplayTagUtilities::IsValidChildTag(
				Line.m_SpeakerID,
				SIHGameplayTags::ID_Character))
			{
				return Fail(FString::Printf(
					TEXT(
						"Speaker ID is outside the Character ID domain. "
						"NodeID=%s, LineID=%s, SpeakerID=%s"),
					*NodeID.ToString(),
					*Line.m_LineID.ToString(),
					*Line.m_SpeakerID.ToString()));
			}

			if (LineIDs.Contains(Line.m_LineID))
			{
				return Fail(FString::Printf(
					TEXT("Line ID is duplicated. LineID=%s"),
					*Line.m_LineID.ToString()));
			}

			LineIDs.Add(Line.m_LineID);
		}

		if (!Node.m_NextNodeID.IsNone()
			&& !m_Nodes.Contains(Node.m_NextNodeID))
		{
			return Fail(FString::Printf(
				TEXT("Next node was not found. NodeID=%s, NextNodeID=%s"),
				*NodeID.ToString(),
				*Node.m_NextNodeID.ToString()));
		}

		for (const FPFDialogueChoice& Choice : Node.m_Choices)
		{
			if (!m_Nodes.Contains(Choice.m_NextNodeID))
			{
				return Fail(FString::Printf(
					TEXT(
						"Choice target node was not found. "
						"NodeID=%s, ChoiceID=%s, NextNodeID=%s"),
					*NodeID.ToString(),
					*Choice.m_ChoiceID.ToString(),
					*Choice.m_NextNodeID.ToString()));
			}
		}

		for (const FPFDialogueActionBranch& Branch : Node.m_Action.m_Branches)
		{
			if (!m_Nodes.Contains(Branch.m_NextNodeID))
			{
				return Fail(FString::Printf(
					TEXT(
						"Action branch target node was not found. "
						"NodeID=%s, ResultID=%s, NextNodeID=%s"),
					*NodeID.ToString(),
					*Branch.m_ResultID.ToString(),
					*Branch.m_NextNodeID.ToString()));
			}
		}
	}

	if (OutError)
	{
		OutError->Reset();
	}
	return true;
}
