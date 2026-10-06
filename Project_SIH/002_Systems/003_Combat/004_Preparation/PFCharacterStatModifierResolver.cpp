#include "PFCharacterStatModifierResolver.h"

#include "Engine/GameInstance.h"
#include "Project_SIH/000_Core/001_Contracts/004_Character/PFCharacterStateTypes.h"
#include "Project_SIH/000_Core/001_Contracts/005_Item/PFItemInstanceTypes.h"
#include "Project_SIH/000_Core/001_Contracts/005_Item/PFItemTypes.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/001_Data/000_Definitions/PFItemDefinition.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"
#include "Project_SIH/002_Systems/004_CharacterState/PFCharacterStateSubsystem.h"
#include "Project_SIH/002_Systems/005_Inventory/PFInventorySubsystem.h"

FPFCharacterStatModifierResolver::
	FPFCharacterStatModifierResolver(
		UGameInstance& GameInstance)
	: m_GameInstance(GameInstance)
{
}

bool FPFCharacterStatModifierResolver::Resolve(
	const FGameplayTag& CharacterID,
	FPFResolvedCharacterStatModifiers&
		OutModifiers) const
{
	OutModifiers =
		FPFResolvedCharacterStatModifiers();

	const UPFCharacterStateSubsystem*
		CharacterStateSubsystem =
			m_GameInstance.GetSubsystem<
				UPFCharacterStateSubsystem>();

	const UPFInventorySubsystem* InventorySubsystem =
		m_GameInstance.GetSubsystem<
			UPFInventorySubsystem>();

	if (!IsValid(CharacterStateSubsystem)
		|| !IsValid(InventorySubsystem))
	{
		PF_LOG(
			TEXT(
				"Subsystem required for character Stat "
				"modifier resolution is unavailable"));
		return false;
	}

	FPFCharacterStateData CharacterState;

	if (!CharacterStateSubsystem
		->TryGetCharacterState(
			CharacterID,
			CharacterState))
	{
		PF_LOG(
			TEXT(
				"CharacterState is unavailable. "
				"CharacterID=%s"),
			*CharacterID.ToString());
		return false;
	}

	FPFResolvedCharacterStatModifiers Result;

	Result.m_CharacterLevel =
		CharacterState.m_ProgressData.m_Level;

	UPFAssetManager& AssetManager =
		UPFAssetManager::Get();

	for (const TPair<FGameplayTag, FGuid>& EquippedItem
		: CharacterState
			.m_EquipmentLoadout
			.m_EquippedItemInstanceIDs)
	{
		const FGuid& ItemInstanceID =
			EquippedItem.Value;

		FPFItemInstanceData ItemInstance;

		if (!InventorySubsystem->TryGetItemInstance(
			ItemInstanceID,
			ItemInstance))
		{
			PF_LOG(
				TEXT(
					"Equipped item instance is unavailable. "
					"ItemInstanceID=%s"),
				*ItemInstanceID.ToString());
			return false;
		}

		const UPFItemDefinition* ItemDefinition =
			AssetManager.GetItemDefinition(
				ItemInstance.m_DefinitionID);

		if (!IsValid(ItemDefinition))
		{
			PF_LOG(
				TEXT(
					"Equipped item definition "
					"is unavailable. DefinitionID=%s"),
				*ItemInstance.m_DefinitionID.ToString());
			return false;
		}

		const FPFEquipmentStatModifierFragment*
			StatModifierFragment =
				ItemDefinition->FindFragment<
					FPFEquipmentStatModifierFragment>();

		if (!StatModifierFragment)
		{
			continue;
		}

		Result
			.m_EquipmentModifiers
			.m_FlatModifiers +=
				StatModifierFragment
					->m_StatModifiers
					.m_FlatModifiers;

		Result
			.m_EquipmentModifiers
			.m_PercentModifiers +=
				StatModifierFragment
					->m_StatModifiers
					.m_PercentModifiers;
	}

	OutModifiers = MoveTemp(Result);
	return true;
}
