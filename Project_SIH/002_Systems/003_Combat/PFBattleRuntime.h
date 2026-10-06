#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/000_Core/001_Contracts/000_Flow/PFGameFlowTypes.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleTypes.h"
#include "UObject/Object.h"
#include "PFBattleRuntime.generated.h"

class APFBattleCharacterBase;
class AActor;
class UPFTurnSystem;

UCLASS()
class PROJECT_SIH_API UPFBattleRuntime : public UObject
{
	GENERATED_BODY()

public:
	void Init();
	void Deinit();

	EPFPhaseStartResult StartBattle(
		const FPFBattleRuntimeContext& Context);

	bool RequestBattleAction(
		const FPFBattleActionRequest& Request);

	bool GetValidTargetsAndDefaultTarget(
		APFBattleCharacterBase* Requester,
		FGameplayTag ActionTag,
		FPFBattleTargetSelection& OutSelection) const;

	void CompleteBattle(EPFBattleResult ResultType);

	bool SelectBattleAction(
		APFBattleCharacterBase* Requester,
		FGameplayTag ActionTag);

	bool SelectBattleTarget(
		APFBattleCharacterBase* Requester,
		APFBattleCharacterBase* Target);

	bool ConfirmBattleAction(APFBattleCharacterBase* Requester);

	bool CancelBattleActionSelection(APFBattleCharacterBase* Requester);

private:
	bool ValidateSelectionRequester(APFBattleCharacterBase* Requester) const;

	void ClearActionSelection();

	void CollectValidTargets(
		APFBattleCharacterBase* Requester,
		FGameplayTag ActionTag,
		const FPFBattleTargetRule& Rule,
		TArray<TObjectPtr<APFBattleCharacterBase>>& OutTargets) const;

	void HandleBattleActionCompleted(
		AActor* ActionOwner);

	void HandleBattleProgressionReady();

	void RequestActionSelection();

	void BroadcastActionState(FGameplayTag Channel);

	void BindActionCompletionDelegates();

	void UnbindActionCompletionDelegates();

	bool DetermineBattleResult(
		EPFBattleResult& OutResult) const;

private:
	FGameplayTag m_CaseID;
	FGameplayTag m_SelectedActionTag;

	UPROPERTY()
	FPFBattleTargetSelection m_TargetSelection;

	UPROPERTY()
	TArray<TObjectPtr<APFBattleCharacterBase>>
		m_Participants;

	UPROPERTY()
	TObjectPtr<UPFTurnSystem> m_TurnSystem;

	// 전투 시작 때 정한 승리 유형이며, 실제 종료 결과와는 별개다.
	EPFBattleResult m_ResultOnVictory =
		EPFBattleResult::NormalVictory;

	EPFBattleResult m_FinalResult =
		EPFBattleResult::None;

	TMap<TWeakObjectPtr<AActor>, FDelegateHandle>
		m_ActionCompletionBindings;
};
