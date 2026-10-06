#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "PFDialogueGraphTypes.generated.h"

UENUM(BlueprintType)
enum class EPFDialogueRowType : uint8
{
	Dialogue,
	Choice UMETA(DisplayName = "Inline Choice")
};

UENUM(BlueprintType)
enum class EPFDialogueGraphNodeType : uint8
{
	Root,
	Dialogue,
	InlineChoice,
	ClaimSelectionRequest,
	EvidenceSelectionRequest,
	End
};

USTRUCT()
struct PROJECT_SIH_API FPFDialogueGraphTransition
{
	GENERATED_BODY()

	UPROPERTY()
	FName OutputName;

	UPROPERTY()
	FGuid TargetNodeID;
};

/** Cooked/runtime representation generated from the editor graph on asset save. */
USTRUCT()
struct PROJECT_SIH_API FPFDialogueRuntimeNode
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid NodeID;

	UPROPERTY()
	EPFDialogueGraphNodeType NodeType = EPFDialogueGraphNodeType::Dialogue;

	UPROPERTY()
	FName RowName;

	/** Claim options for a ClaimSelectionRequest node. */
	UPROPERTY()
	TArray<FGameplayTag> OptionContentIDs;

	/** Claim being supported by an EvidenceSelectionRequest node. */
	UPROPERTY()
	FGameplayTag ContextContentID;

	UPROPERTY()
	TArray<FPFDialogueGraphTransition> Transitions;
};

/**
 * Authoring row used by the dialogue graph prototype.
 * Dialogue rows use Speaker/Text, while Inline Choice rows use Text/Choices.
 */
USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	EPFDialogueRowType RowType = EPFDialogueRowType::Dialogue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (MultiLine = true))
	FText Text;

	/** Editable only for Choice rows. Each entry produces one output pin on a Choice node. */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Choice",
		meta = (EditCondition = "RowType == EPFDialogueRowType::Choice"))
	TArray<FText> Choices;

	/** Serialized only to preserve DataTables created before dynamic choices were added. */
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use Choices."))
	FText ChoiceA;

	/** Serialized only to preserve DataTables created before dynamic choices were added. */
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use Choices."))
	FText ChoiceB;
};
