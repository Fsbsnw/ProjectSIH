#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/000_Core/001_Contracts/005_Item/PFItemInstanceTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "PFInventorySubsystem.generated.h"

UCLASS()
class PROJECT_SIH_API UPFInventorySubsystem
	: public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool TryCreateItemInstance(
		const FPFItemInstanceData& InitialData,
		FGuid& OutItemInstanceID);

	bool TryGetItemInstance(
		const FGuid& ItemInstanceID,
		FPFItemInstanceData& OutItemInstance) const;

	bool TryUpdateItemInstanceState(
		const FGuid& ItemInstanceID,
		const TInstancedStruct<FPFItemInstanceState>& InstanceState);

	bool TryRemoveItemInstance(
		const FGuid& ItemInstanceID);

private:
	UPROPERTY()
	TMap<FGuid, FPFItemInstanceData> m_ItemInstances;
};
