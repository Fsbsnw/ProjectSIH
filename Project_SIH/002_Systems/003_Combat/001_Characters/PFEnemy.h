#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "TimerManager.h"
#include "Project_SIH/002_Systems/003_Combat/001_Characters/PFBattleCharacterBase.h"
#include "PFEnemy.generated.h"

struct FPFBattleActionStateMessage;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API APFEnemy : public APFBattleCharacterBase
{
	GENERATED_BODY()

public:
	virtual EPFBattleSide GetBattleSide() const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleActionState(
		FGameplayTag Channel,
		const FPFBattleActionStateMessage& Message);

	// 현재 기본공격을 선택한다.
	// 추후 AI(StateTree 등)로 행동·대상 결정과 확정 책임을 이전한다.
	void SelectAction();
	void ConfirmAction();

	FGameplayMessageListenerHandle m_ActionStateHandle;
	FTimerHandle m_ActionSelectionTimer;
};
