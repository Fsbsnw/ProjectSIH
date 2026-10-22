#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PFDialogueTypes.generated.h"

/** 외부 대사 원본에서 가져오는 한 줄의 런타임 데이터입니다. */
USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Line ID"))
	FName m_LineID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Speaker ID", Categories = "ID.Character"))
	FGameplayTag m_SpeakerID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Text"))
	FText m_Text;

	/** 비어 있으면 Speaker의 기본 초상화를 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Portrait Variant"))
	FName m_PortraitVariant;

	bool IsValid() const
	{
		return !m_LineID.IsNone()
			&& m_SpeakerID.IsValid()
			&& !m_Text.IsEmpty();
	}
};

/** 선택지 표시 데이터와 선택 후 이동할 노드를 함께 정의합니다. */
USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Choice ID"))
	FName m_ChoiceID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Text"))
	FText m_Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Next Node ID"))
	FName m_NextNodeID;

	bool IsValid() const
	{
		return !m_ChoiceID.IsNone()
			&& !m_Text.IsEmpty()
			&& !m_NextNodeID.IsNone();
	}
};

/** Action 실행 결과와 해당 결과가 이동할 노드를 연결합니다. */
USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueActionBranch
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Result ID"))
	FName m_ResultID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Next Node ID"))
	FName m_NextNodeID;

	bool IsValid() const
	{
		return !m_ResultID.IsNone()
			&& !m_NextNodeID.IsNone();
	}
};

/** 비동기 실행 주체에게 전달할 Action과 결과별 분기 계약입니다. */
USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Action ID"))
	FGameplayTag m_ActionID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Branches"))
	TArray<FPFDialogueActionBranch> m_Branches;

	bool IsValid() const
	{
		if (!m_ActionID.IsValid() || m_Branches.IsEmpty())
		{
			return false;
		}

		TSet<FName> ResultIDs;
		for (const FPFDialogueActionBranch& Branch : m_Branches)
		{
			if (!Branch.IsValid()
				|| ResultIDs.Contains(Branch.m_ResultID))
			{
				return false;
			}

			ResultIDs.Add(Branch.m_ResultID);
		}

		return true;
	}
};

UENUM(BlueprintType)
enum class EPFDialogueNodeType : uint8
{
	Dialogue,
	Action,
	End,
};

/** 외부 그래프가 Export하는 실행 노드입니다. */
USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueRuntimeNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Node Type"))
	EPFDialogueNodeType m_NodeType = EPFDialogueNodeType::Dialogue;

	/** Dialogue 노드에서 순서대로 표시할 대사입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Lines"))
	TArray<FPFDialogueLine> m_Lines;

	/** 마지막 대사 뒤에 표시할 선택지입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Choices"))
	TArray<FPFDialogueChoice> m_Choices;

	/** 선택지가 없는 Dialogue 노드의 다음 노드입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Next Node ID"))
	FName m_NextNodeID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (DisplayName = "Action"))
	FPFDialogueAction m_Action;

	bool IsStructurallyValid() const;
};

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueLinePresentation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly,
		meta = (DisplayName = "Line"))
	FPFDialogueLine m_Line;

	UPROPERTY(BlueprintReadOnly,
		meta = (DisplayName = "Line Index"))
	int32 m_LineIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly,
		meta = (DisplayName = "Line Count"))
	int32 m_LineCount = 0;
};

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueChoicePresentation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly,
		meta = (DisplayName = "Choices"))
	TArray<FPFDialogueChoice> m_Choices;
};

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueActionPresentation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly,
		meta = (DisplayName = "Node ID"))
	FName m_NodeID;

	UPROPERTY(BlueprintReadOnly,
		meta = (DisplayName = "Action"))
	FPFDialogueAction m_Action;
};

UENUM(BlueprintType)
enum class EPFDialogueInputResult : uint8
{
	Accepted,
	InactiveDialogue,
	AlreadyActive,
	InvalidDialogueDefinition,
	InvalidNodeID,
	WrongStep,
	InvalidChoiceID,
	StaleChoice,
	StaleAction,
	InvalidActionResultID,
	ActionBranchNotFound,
	ReentrantInput,
};
