#pragma once

#include "CommonUserWidget.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Styling/SlateBrush.h"
#include "PFPartyCandidateEntryWidget.generated.h"

class UBorder;
class UCommonButtonBase;
class UCommonTextBlock;
class UImage;
class UTexture2D;

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API UPFPartyCandidateEntryWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FCandidateClickedEvent, const FGameplayTag&);

	void SetCandidate(
		FGameplayTag CharacterID,
		const FText& DisplayName,
		int32 Level,
		UTexture2D* Icon);

	void SetSelected(bool bSelected);

	FCandidateClickedEvent& OnCandidateClicked();

protected:
	virtual void NativeOnInitialized() override;

private:
	void HandleClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Candidate;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_CharacterName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Level;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_CharacterIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_Selected;

	FGameplayTag m_CharacterID;

	UPROPERTY(Transient)
	FSlateBrush m_DefaultIconBrush;

	FCandidateClickedEvent m_OnCandidateClicked;
};
