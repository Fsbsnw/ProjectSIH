#include "PFInventorySubsystem.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"

bool UPFInventorySubsystem::TryCreateItemInstance(
	const FPFItemInstanceData& InitialData,
	FGuid& OutItemInstanceID)
{
	OutItemInstanceID = FGuid();

	if (!InitialData.IsValid())
	{
		PF_LOG(
			TEXT(
				"Initial item instance data is invalid. "
				"DefinitionID=%s"),
			*InitialData.m_DefinitionID.ToString());
		return false;
	}

	UPFAssetManager& AssetManager = UPFAssetManager::Get();

	if (!AssetManager.GetItemDefinition(InitialData.m_DefinitionID))
	{
		PF_LOG(
			TEXT("Item definition is unavailable. DefinitionID=%s"),
			*InitialData.m_DefinitionID.ToString());
		return false;
	}

	const FGuid NewItemInstanceID = FGuid::NewGuid();

	m_ItemInstances.Add(NewItemInstanceID, InitialData);
	OutItemInstanceID = NewItemInstanceID;

	return true;
}

bool UPFInventorySubsystem::TryGetItemInstance(
	const FGuid& ItemInstanceID,
	FPFItemInstanceData& OutItemInstance) const
{
	if (!ItemInstanceID.IsValid())
	{
		PF_LOG(TEXT("ItemInstanceID is invalid."));
		return false;
	}

	const FPFItemInstanceData* FoundItemInstance =
		m_ItemInstances.Find(ItemInstanceID);

	if (!FoundItemInstance)
	{
		PF_LOG(
			TEXT("Item instance was not found. ItemInstanceID=%s"),
			*ItemInstanceID.ToString());
		return false;
	}

	OutItemInstance = *FoundItemInstance;
	return true;
}

bool UPFInventorySubsystem::TryUpdateItemInstanceState(
	const FGuid& ItemInstanceID,
	const TInstancedStruct<FPFItemInstanceState>& InstanceState)
{
	if (!ItemInstanceID.IsValid())
	{
		PF_LOG(TEXT("ItemInstanceID is invalid."));
		return false;
	}

	FPFItemInstanceData* FoundItemInstance =
		m_ItemInstances.Find(ItemInstanceID);

	if (!FoundItemInstance)
	{
		PF_LOG(
			TEXT("Item instance was not found. ItemInstanceID=%s"),
			*ItemInstanceID.ToString());
		return false;
	}

	FoundItemInstance->m_InstanceState = InstanceState;
	return true;
}

bool UPFInventorySubsystem::TryRemoveItemInstance(
	const FGuid& ItemInstanceID)
{
	if (!ItemInstanceID.IsValid())
	{
		PF_LOG(TEXT("ItemInstanceID is invalid."));
		return false;
	}

	if (m_ItemInstances.Remove(ItemInstanceID) == 0)
	{
		PF_LOG(
			TEXT("Item instance was not found. ItemInstanceID=%s"),
			*ItemInstanceID.ToString());
		return false;
	}

	return true;
}
