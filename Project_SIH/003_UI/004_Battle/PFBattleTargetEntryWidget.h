#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PFBattleTargetEntryWidget.generated.h"

class APFBattleCharacterBase;
class UBorder;
class UCommonButtonBase;
class UCommonTextBlock;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFBattleTargetEntryWidget
	: public UCommonUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(
		FTargetClickedEvent, APFBattleCharacterBase*);

	void SetTarget(
		APFBattleCharacterBase* Target,
		bool bCanSelect);

	void SetSelected(bool bSelected);

	APFBattleCharacterBase* GetTarget() const;

	FTargetClickedEvent& OnTargetClicked();

protected:
	virtual void NativeOnInitialized() override;

private:
	void HandleClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Target;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_Selected;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_SelectedMarker;

	TWeakObjectPtr<APFBattleCharacterBase> m_Target;

	FTargetClickedEvent m_OnTargetClicked;
};
