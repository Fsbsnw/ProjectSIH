#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayTagContainer.h"
#include "PFBattleCharacterStatusWidget.generated.h"

class APFBattleCharacterBase;
class UAbilitySystemComponent;
class UBorder;
class UCommonTextBlock;
class UProgressBar;
struct FOnAttributeChangeData;
struct FPFBattleActionStateMessage;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFBattleCharacterStatusWidget
	: public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void SetCharacter(
		APFBattleCharacterBase* Character,
		const FText& DisplayName);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void UnbindCharacter();
	void HandleHPChanged(
		const FOnAttributeChangeData& ChangeData);
	void HandleMaxHPChanged(
		const FOnAttributeChangeData& ChangeData);
	void RefreshHP();
	void HandleActionStateChanged(
		FGameplayTag Channel,
		const FPFBattleActionStateMessage& Message);
	void SetActionOwnerHighlighted(bool bHighlighted);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_CurrentActionOwner;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_CharacterName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_HP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_HP;

	TWeakObjectPtr<UAbilitySystemComponent> m_ObservedASC;
	TWeakObjectPtr<APFBattleCharacterBase> m_ObservedCharacter;
	FGameplayMessageListenerHandle m_ActionStateMessageHandle;
	FDelegateHandle m_HPChangedHandle;
	FDelegateHandle m_MaxHPChangedHandle;
	float m_CurrentHP = 0.0f;
	float m_CurrentMaxHP = 0.0f;
};
