#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Project_SIH/002_Systems/008_Character/PFCharacterBase.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFBattleParticipantInterface.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFBattleActionParticipantInterface.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFDeathParticipantInterface.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFTurnParticipantInterface.h"
#include "PFBattleCharacterBase.generated.h"

class UPFAbilityBase;
class UPFAbilitySystemComponent;
class UPFBattleAttributeSet;
class UAbilitySystemComponent;
struct FOnAttributeChangeData;
struct FPFCharacterStatValues;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API APFBattleCharacterBase
	: public APFCharacterBase
	, public IAbilitySystemInterface
	, public IPFBattleParticipantInterface
	, public IPFBattleActionParticipantInterface
	, public IPFDeathParticipantInterface
	, public IPFTurnParticipantInterface
{
	GENERATED_BODY()

public:
	APFBattleCharacterBase();

	virtual UAbilitySystemComponent*
		GetAbilitySystemComponent() const override;

	UPFAbilitySystemComponent*
		GetPFAbilitySystemComponent() const;

	virtual EPFBattleSide GetBattleSide() const override;

	virtual bool IsAlive() const override;

	virtual float GetTurnSpeed() const override;

	virtual bool RequestBattleAction(
		const FPFBattleActionRequest& Request) override;

	virtual void NotifyBattleActionCompleted() override;

	virtual FPFOnBattleActionCompleted&
		GetBattleActionCompletedDelegate() override;

	virtual bool HasPendingDeath() const override;

	virtual bool StartPendingDeath() override;

	virtual void NotifyDeathStarted() override;

	virtual void NotifyDeathFinished() override;

	virtual FPFOnDeathStarted&
		GetDeathStartedDelegate() override;

	virtual FPFOnDeathFinished&
		GetDeathFinishedDelegate() override;

	FPFOnBattleActionCompleted m_OnBattleActionCompleted;
	FPFOnDeathStarted m_OnDeathStarted;
	FPFOnDeathFinished m_OnDeathFinished;

	bool TryInitializeBattleAttributes(
		const FPFCharacterStatValues& InitialStats);

	bool TryGrantInitialAbilities(
		const TArray<TSubclassOf<UPFAbilityBase>>&
			AbilityClasses);

protected:
	virtual void BeginPlay() override;

private:
	void HandleHPChanged(
		const FOnAttributeChangeData& ChangeData);

	bool ValidateInitialAbilityClasses(
		const TArray<TSubclassOf<UPFAbilityBase>>&
			AbilityClasses) const;

	void GrantInitialAbility(
		TSubclassOf<UPFAbilityBase> AbilityClass);

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Battle|GAS",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPFAbilitySystemComponent> m_AbilitySystemComponent;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Battle|GAS",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPFBattleAttributeSet> m_BattleAttributeSet;
};
