#include "PFBattleWeaknessSlotsWidget.h"

#include "CommonTextBlock.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleMessages.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/SIHGameplayTags.h"

namespace
{
	FText GetWeaknessLabel(FGameplayTag ElementID)
	{
		if (!ElementID.IsValid())
		{
			return NSLOCTEXT(
				"PFBattleWeaknessSlotsWidget",
				"UnknownWeakness",
				"?");
		}

		if (ElementID == SIHGameplayTags::ID_Element_CheckIn.GetTag())
		{
			return NSLOCTEXT(
				"PFBattleWeaknessSlotsWidget",
				"CheckIn",
				"수속");
		}

		if (ElementID == SIHGameplayTags::ID_Element_Maintenance.GetTag())
		{
			return NSLOCTEXT(
				"PFBattleWeaknessSlotsWidget",
				"Maintenance",
				"정비");
		}

		if (ElementID == SIHGameplayTags::ID_Element_Security.GetTag())
		{
			return NSLOCTEXT(
				"PFBattleWeaknessSlotsWidget",
				"Security",
				"보안");
		}

		if (ElementID == SIHGameplayTags::ID_Element_Guidance.GetTag())
		{
			return NSLOCTEXT(
				"PFBattleWeaknessSlotsWidget",
				"Guidance",
				"안내");
		}

		PF_LOG(
			TEXT("Visible weakness ElementID has no display label. ElementID=%s"),
			*ElementID.ToString());
		return FText::FromString(ElementID.ToString());
	}
}

void UPFBattleWeaknessSlotsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	m_WeaknessMessageHandle =
		UGameplayMessageSubsystem::Get(this)
			.RegisterListener<FPFBattleWeaknessSlotsMessage>(
				SIHGameplayTags::Message_Battle_WeaknessSlots_Initialized.GetTag(),
				this,
				&ThisClass::HandleWeaknessSlotsInitialized);
}

void UPFBattleWeaknessSlotsWidget::NativeDestruct()
{
	m_WeaknessMessageHandle.Unregister();
	Super::NativeDestruct();
}

void UPFBattleWeaknessSlotsWidget::HandleWeaknessSlotsInitialized(
	FGameplayTag Channel,
	const FPFBattleWeaknessSlotsMessage& Message)
{
	if (Message.m_VisibleElementIDs.Num()
		!= FPFInitialWeaknessCounts::SlotCount)
	{
		PF_LOG(TEXT("Battle weakness slot count is invalid."));
		return;
	}

	SetSlots(Message.m_VisibleElementIDs);
}

void UPFBattleWeaknessSlotsWidget::SetSlots(
	const TArray<FGameplayTag>& VisibleElementIDs)
{
	UCommonTextBlock* SlotTexts[] = {
		Text_Weakness1,
		Text_Weakness2,
		Text_Weakness3,
		Text_Weakness4
	};

	for (int32 SlotIndex = 0;
		SlotIndex < FPFInitialWeaknessCounts::SlotCount;
		++SlotIndex)
	{
		if (!IsValid(SlotTexts[SlotIndex]))
		{
			PF_LOG(
				TEXT("Battle weakness text is not bound. SlotIndex=%d"),
				SlotIndex);
			return;
		}

		SlotTexts[SlotIndex]->SetText(
			GetWeaknessLabel(VisibleElementIDs[SlotIndex]));
	}
}
