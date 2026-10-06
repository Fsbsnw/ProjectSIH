#include "PFCharacterStateSubsystem.h"

#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/001_Data/000_Definitions/PFCharacterDefinition.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"

TArray<FGameplayTag> UPFCharacterStateSubsystem::GetCharacterIDs() const
{
	TArray<FGameplayTag> CharacterIDs;
	m_CharacterStates.GetKeys(CharacterIDs);

	CharacterIDs.Sort(
		[](const FGameplayTag& A, const FGameplayTag& B)
		{
			return A.ToString() < B.ToString();
		});

	return CharacterIDs;
}

bool UPFCharacterStateSubsystem::TryCreateCharacterState(
	const FGameplayTag& CharacterID,
	const FPFCharacterStateData& InitialState)
{
	if (!InitialState.IsValid())
	{
		PF_LOG(
			TEXT("Initial character state is invalid. CharacterID=%s, Level=%d"),
			*CharacterID.ToString(),
			InitialState.m_ProgressData.m_Level);
		return false;
	}

	UPFAssetManager& AssetManager = UPFAssetManager::Get();
	const UPFCharacterDefinition* CharacterDefinition =
		AssetManager.GetCharacterDefinition(CharacterID);

	if (!CharacterDefinition)
	{
		PF_LOG(
			TEXT("Character definition is unavailable. CharacterID=%s"),
			*CharacterID.ToString());
		return false;
	}

	if (m_CharacterStates.Contains(CharacterID))
	{
		PF_LOG(
			TEXT("Character state already exists. CharacterID=%s"),
			*CharacterID.ToString());
		return false;
	}

	m_CharacterStates.Add(CharacterID, InitialState);
	return true;
}

bool UPFCharacterStateSubsystem::TryGetCharacterState(
	const FGameplayTag& CharacterID,
	FPFCharacterStateData& OutState) const
{
	if (!UPFCharacterDefinition::MakePrimaryAssetID(CharacterID).IsValid())
	{
		return false;
	}

	const FPFCharacterStateData* FoundState =
		m_CharacterStates.Find(CharacterID);

	if (!FoundState)
	{
		PF_LOG(
			TEXT("Character state was not found. CharacterID=%s"),
			*CharacterID.ToString());
		return false;
	}

	OutState = *FoundState;
	return true;
}

bool UPFCharacterStateSubsystem::TryUpdateCharacterProgress(
	const FGameplayTag& CharacterID,
	const FPFCharacterProgressData& ProgressData)
{
	if (!UPFCharacterDefinition::MakePrimaryAssetID(CharacterID).IsValid())
	{
		return false;
	}

	if (!ProgressData.IsValid())
	{
		PF_LOG(
			TEXT("Character progress is invalid. CharacterID=%s, Level=%d"),
			*CharacterID.ToString(),
			ProgressData.m_Level);
		return false;
	}

	FPFCharacterStateData* FoundState =
		m_CharacterStates.Find(CharacterID);

	if (!FoundState)
	{
		PF_LOG(
			TEXT("Character state was not found. CharacterID=%s"),
			*CharacterID.ToString());
		return false;
	}

	FoundState->m_ProgressData = ProgressData;
	return true;
}

bool UPFCharacterStateSubsystem::TryUpdateEquipmentLoadout(
	const FGameplayTag& CharacterID,
	const FPFEquipmentLoadout& EquipmentLoadout)
{
	if (!UPFCharacterDefinition::MakePrimaryAssetID(CharacterID).IsValid())
	{
		return false;
	}

	if (!EquipmentLoadout.IsValid())
	{
		PF_LOG(
			TEXT(
				"Equipment loadout is invalid. "
				"CharacterID=%s, EquippedItems=%d"),
			*CharacterID.ToString(),
			EquipmentLoadout.m_EquippedItemInstanceIDs.Num());
		return false;
	}

	FPFCharacterStateData* FoundState =
		m_CharacterStates.Find(CharacterID);

	if (!FoundState)
	{
		PF_LOG(
			TEXT("Character state was not found. CharacterID=%s"),
			*CharacterID.ToString());
		return false;
	}

	FoundState->m_EquipmentLoadout = EquipmentLoadout;
	return true;
}
