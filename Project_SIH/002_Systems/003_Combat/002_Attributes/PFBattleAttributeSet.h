#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "PFBattleAttributeSet.generated.h"

#define PF_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

struct FGameplayEffectModCallbackData;

UCLASS()
class PROJECT_SIH_API UPFBattleAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void PostGameplayEffectExecute(
		const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Stat")
	FGameplayAttributeData Attack;
	PF_ATTRIBUTE_ACCESSORS(UPFBattleAttributeSet, Attack)

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Stat")
	FGameplayAttributeData PhysicalDefense;
	PF_ATTRIBUTE_ACCESSORS(UPFBattleAttributeSet, PhysicalDefense)

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Stat")
	FGameplayAttributeData MagicalDefense;
	PF_ATTRIBUTE_ACCESSORS(UPFBattleAttributeSet, MagicalDefense)

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Stat")
	FGameplayAttributeData TurnSpeed;
	PF_ATTRIBUTE_ACCESSORS(
		UPFBattleAttributeSet,
		TurnSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Resource")
	FGameplayAttributeData HP;
	PF_ATTRIBUTE_ACCESSORS(UPFBattleAttributeSet, HP)

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Resource")
	FGameplayAttributeData MaxHP;
	PF_ATTRIBUTE_ACCESSORS(UPFBattleAttributeSet, MaxHP)

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Resource")
	FGameplayAttributeData UltimateGauge;
	PF_ATTRIBUTE_ACCESSORS(UPFBattleAttributeSet, UltimateGauge)

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Resource")
	FGameplayAttributeData MaxUltimateGauge;
	PF_ATTRIBUTE_ACCESSORS(
		UPFBattleAttributeSet,
		MaxUltimateGauge)
};

#undef PF_ATTRIBUTE_ACCESSORS
