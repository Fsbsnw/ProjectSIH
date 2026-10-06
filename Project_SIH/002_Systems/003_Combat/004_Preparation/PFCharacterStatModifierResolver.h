#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/000_Core/001_Contracts/004_Character/PFCharacterStatTypes.h"

class UGameInstance;

struct FPFResolvedCharacterStatModifiers
{
	int32 m_CharacterLevel = 1;

	FPFCharacterStatModifiers
		m_EquipmentModifiers;
};

class PROJECT_SIH_API FPFCharacterStatModifierResolver
{
public:
	FPFCharacterStatModifierResolver(
		UGameInstance& GameInstance);

	bool Resolve(
		const FGameplayTag& CharacterID,
		FPFResolvedCharacterStatModifiers&
			OutModifiers) const;

private:
	UGameInstance& m_GameInstance;
};
