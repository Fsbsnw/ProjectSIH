#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/PFBattleActionCommandReceiver.h"
#include "Interfaces/PFBattleEntryReceiver.h"
#include "PFBattleGameMode.generated.h"

class ACameraActor;
class UPFBattleRuntime;
class UPFBattleSpawner;
struct FPFBattleSpawnLayout;

UCLASS()
class PROJECT_SIH_API APFBattleGameMode
	: public AGameModeBase
	, public IPFBattleEntryReceiver
	, public IPFBattleActionCommandReceiver
{
	GENERATED_BODY()

public:
	APFBattleGameMode();

	virtual EPFPhaseStartResult StartBattle(
		const FPFBattleEntryContext& Context) override;

	virtual bool SelectBattleAction(
		APFBattleCharacterBase* Requester,
		FGameplayTag ActionTag) override;

	virtual bool SelectBattleTarget(
		APFBattleCharacterBase* Requester,
		APFBattleCharacterBase* Target) override;

	virtual bool ConfirmBattleAction(
		APFBattleCharacterBase* Requester) override;

	virtual bool CancelBattleActionSelection(
		APFBattleCharacterBase* Requester) override;

	virtual bool GetValidTargetsAndDefaultTarget(
		APFBattleCharacterBase* Requester,
		FGameplayTag ActionTag,
		FPFBattleTargetSelection& OutSelection) const override;

	virtual void InitGame(
		const FString& MapName,
		const FString& Options,
		FString& ErrorMessage) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason) override;

private:
	bool TryResolveBattleMapSetup(
		FPFBattleSpawnLayout& OutLayout,
		ACameraActor*& OutCamera) const;

	void SendBattleReadyMessage();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Battle|Debug",
		meta = (
			DisplayName = "Revealed Weakness Count For Debug",
			ClampMin = "-1",
			ClampMax = "4"))
	int32 m_RevealedWeaknessCountForDebug = -1;

	UPROPERTY(Transient)
	TObjectPtr<UPFBattleSpawner> m_BattleSpawner;

	UPROPERTY(Transient)
	TObjectPtr<UPFBattleRuntime> m_BattleRuntime;
};
