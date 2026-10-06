#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleTypes.h"
#include "PFBattleActionSelectionWidget.generated.h"

class APFBattleCharacterBase;
class UPFBattleActionPanelWidget;
class UPFBattleTargetIndicatorWidget;
class IPFBattleActionCommandReceiver;
struct FPFBattleActionStateMessage;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFBattleActionSelectionWidget
	: public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void HandleActionState(
		FGameplayTag Channel,
		const FPFBattleActionStateMessage& Message);

	void HandleBasicAttackClicked();
	void HandleTargetClicked(APFBattleCharacterBase* Target);
	void HandleCancelClicked();
	void ClearSelection();
	void RefreshControls();
	IPFBattleActionCommandReceiver* GetCommandReceiver() const;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPFBattleActionPanelWidget> ActionPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPFBattleTargetIndicatorWidget> TargetIndicator;

	FGameplayMessageListenerHandle m_ActionStateHandle;

	TWeakObjectPtr<APFBattleCharacterBase> m_ActionOwner;
	EPFTargetCount m_TargetCount = EPFTargetCount::Single;

	bool m_bCanPlayerSelectAction = false;
	bool m_bSelectingTarget = false;
};
