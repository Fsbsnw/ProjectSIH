#include "PFDialogueTypes.h"

bool FPFDialogueRuntimeNode::IsStructurallyValid() const
{
	switch (m_NodeType)
	{
	case EPFDialogueNodeType::Dialogue:
	{
		if (m_Lines.IsEmpty() && m_Choices.IsEmpty())
		{
			return false;
		}

		for (const FPFDialogueLine& Line : m_Lines)
		{
			if (!Line.IsValid())
			{
				return false;
			}
		}

		TSet<FName> ChoiceIDs;
		for (const FPFDialogueChoice& Choice : m_Choices)
		{
			if (!Choice.IsValid()
				|| ChoiceIDs.Contains(Choice.m_ChoiceID))
			{
				return false;
			}

			ChoiceIDs.Add(Choice.m_ChoiceID);
		}

		return m_Choices.IsEmpty()
			? !m_NextNodeID.IsNone()
			: m_NextNodeID.IsNone();
	}

	case EPFDialogueNodeType::Action:
		return m_Action.IsValid()
			&& m_NextNodeID.IsNone()
			&& m_Lines.IsEmpty()
			&& m_Choices.IsEmpty();

	case EPFDialogueNodeType::End:
		return m_Lines.IsEmpty()
			&& m_Choices.IsEmpty()
			&& m_NextNodeID.IsNone()
			&& !m_Action.IsValid();
	}

	return false;
}
