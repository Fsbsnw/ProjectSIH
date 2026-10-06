#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PFDialogueRuntimeNodeProvider.generated.h"

struct FPFDialogueRuntimeNode;

UINTERFACE(MinimalAPI)
class UPFDialogueRuntimeNodeProvider : public UInterface
{
	GENERATED_BODY()
};

/** Editor graph nodes implement this without creating a Runtime -> Editor module dependency. */
class PROJECT_SIH_API IPFDialogueRuntimeNodeProvider
{
	GENERATED_BODY()

public:
	virtual bool BuildRuntimeNode(FPFDialogueRuntimeNode& OutNode) const { return false; }
};
