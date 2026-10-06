#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PFDialogueContracts.generated.h"

/** External selection owned by another UI or gameplay system. */
UENUM(BlueprintType)
enum class EPFDialogueExternalRequestType : uint8
{
	Claim,
	Evidence
};

/** Normalized response used to resume a suspended dialogue graph. */
UENUM(BlueprintType)
enum class EPFDialogueExternalResponseType : uint8
{
	Selected,
	Correct,
	Incorrect,
	Cancelled,
	Failed
};

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueLinePresentation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FName Speaker;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FText Text;
};

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueInlineChoicePresentation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FText Text;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TArray<FText> Choices;
};

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueExternalSelectionRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FGuid RequestID;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	EPFDialogueExternalRequestType RequestType = EPFDialogueExternalRequestType::Claim;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FText Prompt;

	/** Claim candidates for Claim requests. Evidence candidates remain owned by MeetingSystem. */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TArray<FGameplayTag> CandidateContentIDs;

	/** Selected ClaimID that an Evidence request is asking the player to support. */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FGameplayTag ContextContentID;
};

USTRUCT(BlueprintType)
struct PROJECT_SIH_API FPFDialogueExternalSelectionResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FGuid RequestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	EPFDialogueExternalResponseType ResponseType = EPFDialogueExternalResponseType::Failed;

	/** ClaimID for Claim requests or ClueID for Evidence requests. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FGameplayTag SelectedContentID;
};
