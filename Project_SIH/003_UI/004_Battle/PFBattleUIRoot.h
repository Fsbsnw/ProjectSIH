#pragma once

#include <Project_SIH/003_UI/000_Foundation/PFGameUIRootBase.h>

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayTagContainer.h"
#include "PFBattleUIRoot.generated.h"

class UPFBattleCharacterStatusWidget;
struct FPFBattleParticipantEntry;
struct FPFBattleParticipantsInitializedMessage;
struct FPFBattleResult;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFBattleUIRoot : public UPFGameUIRootBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void HandleParticipantsInitialized(
		FGameplayTag Channel,
		const FPFBattleParticipantsInitializedMessage& Message);

	void HandleBattleCompleted(
		FGameplayTag Channel,
		const FPFBattleResult& Result);

	void SetStatusWidget(
		UPFBattleCharacterStatusWidget* StatusWidget,
		const FPFBattleParticipantEntry& Entry);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPFBattleCharacterStatusWidget> Status_Boss;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPFBattleCharacterStatusWidget> Status_Party01;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPFBattleCharacterStatusWidget> Status_Party02;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPFBattleCharacterStatusWidget> Status_Party03;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPFBattleCharacterStatusWidget> Status_Party04;

	FGameplayMessageListenerHandle m_ParticipantsMessageHandle;
	FGameplayMessageListenerHandle m_BattleCompletedHandle;
};
