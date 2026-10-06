#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project_SIH/003_UI/000_Foundation/PFActivatableWidget.h"
#include "PFPartyFormationScreen.generated.h"

class UCommonButtonBase;
class UCommonTextBlock;
class UHorizontalBox;
class UScrollBox;
class UPFPartyCandidateEntryWidget;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFPartyFormationScreen : public UPFActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeOnActivated() override;

	UFUNCTION(BlueprintCallable, Category = "UI|Party",
		meta = (BlueprintProtected = "true"))
	bool TryGetCandidateCharacterIDs(
		TArray<FGameplayTag>& OutCharacterIDs) const;

private:
	void RefreshCandidateList();

	void HandleCandidateClicked(const FGameplayTag& CharacterID);

	void RefreshSelectedCount();

	void HandleDeployClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox_Candidates;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HorizontalBox_Slots;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_SelectedCount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Deploy;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Party",
		meta = (DisplayName = "Candidate Entry Class"))
	TSubclassOf<UPFPartyCandidateEntryWidget> m_CandidateEntryClass;

	TArray<FGameplayTag> m_SelectedCharacterIDs;
};
