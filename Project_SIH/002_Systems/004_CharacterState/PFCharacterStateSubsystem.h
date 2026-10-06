#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/004_Character/PFCharacterStateTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PFCharacterStateSubsystem.generated.h"

UCLASS()
class PROJECT_SIH_API UPFCharacterStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	TArray<FGameplayTag> GetCharacterIDs() const;

	bool TryCreateCharacterState(
		const FGameplayTag& CharacterID,
		const FPFCharacterStateData& InitialState);

	bool TryGetCharacterState(
		const FGameplayTag& CharacterID,
		FPFCharacterStateData& OutState) const;

	bool TryUpdateCharacterProgress(
		const FGameplayTag& CharacterID,
		const FPFCharacterProgressData& ProgressData);

	bool TryUpdateEquipmentLoadout(
		const FGameplayTag& CharacterID,
		const FPFEquipmentLoadout& EquipmentLoadout);

private:
	TMap<FGameplayTag, FPFCharacterStateData> m_CharacterStates;
};
