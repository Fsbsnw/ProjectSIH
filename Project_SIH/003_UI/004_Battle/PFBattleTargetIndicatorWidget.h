#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleTypes.h"
#include "PFBattleTargetIndicatorWidget.generated.h"

class APFBattleCharacterBase;
class UCanvasPanel;
class UPFBattleTargetEntryWidget;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFBattleTargetIndicatorWidget
	: public UCommonUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(
		FTargetClickedEvent, APFBattleCharacterBase*);

	void ShowTargets(
		const FPFBattleTargetSelection& Selection,
		bool bAllowTargetSelection = true);
	void SetSelectedTarget(APFBattleCharacterBase* Target);

	void ClearTargets();

	void RefreshTargetPositions();

	FTargetClickedEvent& OnTargetClicked();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void HandleTargetClicked(APFBattleCharacterBase* Target);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> Canvas_Targets;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Battle",
		meta = (DisplayName = "Target Entry Class"))
	TSubclassOf<UPFBattleTargetEntryWidget> m_TargetEntryClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Battle",
		meta = (DisplayName = "Target World Offset"))
	FVector m_TargetWorldOffset = FVector(0.0, 0.0, 50.0);

	UPROPERTY()
	TArray<TObjectPtr<UPFBattleTargetEntryWidget>> m_Entries;

	FTargetClickedEvent m_OnTargetClicked;
};
