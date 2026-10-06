#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PFTurnSystem.generated.h"

class APFBattleCharacterBase;
class AActor;

DECLARE_MULTICAST_DELEGATE(
	FPFOnBattleProgressionReady);

enum class EPFTurnFlowState : uint8
{
	NotStarted,
	TurnStartProcessing,
	ActionSelection,
	ActionProcessing,
	TurnEndProcessing,
};

UCLASS()
class PROJECT_SIH_API UPFTurnSystem : public UObject
{
	GENERATED_BODY()

public:
	void Deinit();

	bool StartManagingTurns(
		const TArray<TObjectPtr<APFBattleCharacterBase>>&
			Participants);

	bool BeginActionProcessing(
		AActor* ActionOwner);

	void RestoreActionSelection(
		AActor* ActionOwner);

	void HandleBattleActionCompleted(
		AActor* ActionOwner);

	bool StartNextTurn();

	APFBattleCharacterBase*
		GetCurrentActionOwner() const;

	bool IsWaitingForActionRequest() const;

	FPFOnBattleProgressionReady&
		GetBattleProgressionReadyDelegate();

private:
	bool BuildRegularTurnOrder();

	void BeginCurrentRegularTurn();

	void FlushPendingDeaths();

	void HandleDeathStarted(AActor* Participant);

	void HandleDeathFinished(AActor* Participant);

	void TryFinishActionProcessing();

	void BindDeathDelegates();

	void UnbindDeathDelegates();

private:
	UPROPERTY()
	TArray<TObjectPtr<APFBattleCharacterBase>>
		m_Participants;

	UPROPERTY()
	TArray<TObjectPtr<APFBattleCharacterBase>>
		m_RegularTurnOrder;

	int32 m_CurrentTurnIndex = INDEX_NONE;

	EPFTurnFlowState m_State =
		EPFTurnFlowState::NotStarted;

	FPFOnBattleProgressionReady
		m_OnBattleProgressionReady;

	TSet<TWeakObjectPtr<AActor>> m_ActiveDeaths;

	TMap<TWeakObjectPtr<AActor>, FDelegateHandle>
		m_DeathStartedBindings;

	TMap<TWeakObjectPtr<AActor>, FDelegateHandle>
		m_DeathFinishedBindings;

	bool m_bStartingPendingDeaths = false;
};
