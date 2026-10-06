#include "PFPrimaryGameLayout.h"

#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/SIHGameplayTags.h"

namespace
{
	/** Stack의 현재 활성 위젯을 안전하게 반환합니다. */
	UCommonActivatableWidget* GetActiveWidget(const UCommonActivatableWidgetStack* Stack)
	{
		return IsValid(Stack) ? Stack->GetActiveWidget() : nullptr;
	}
}

void UPFPrimaryGameLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	m_Layers.Reset();
	RegisterLayer(SIHGameplayTags::UI_Layer_Game.GetTag(), m_GameStack);
	RegisterLayer(SIHGameplayTags::UI_Layer_Screen.GetTag(), m_ScreenStack);
	RegisterLayer(SIHGameplayTags::UI_Layer_Modal.GetTag(), m_ModalStack);
}

void UPFPrimaryGameLayout::RegisterLayer(FGameplayTag LayerID, UCommonActivatableWidgetStack* LayerStack)
{
	if (!LayerID.IsValid()
		|| !LayerID.MatchesTag(SIHGameplayTags::UI_Layer.GetTag())
		|| LayerID == SIHGameplayTags::UI_Layer.GetTag())
	{
		PF_LOG(TEXT("Layer ID is outside the UI.Layer domain. LayerID=%s"), *LayerID.ToString());
		return;
	}

	if (!IsValid(LayerStack))
	{
		PF_LOG(TEXT("Layer Stack is invalid. LayerID=%s"), *LayerID.ToString());
		return;
	}

	m_Layers.Add(LayerID, LayerStack);
}

UCommonActivatableWidget* UPFPrimaryGameLayout::PushWidgetToLayer(
	FGameplayTag LayerID,
	TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	UCommonActivatableWidgetStack* LayerStack = FindLayer(LayerID);
	if (!IsValid(LayerStack) || !WidgetClass)
	{
		return nullptr;
	}

	return LayerStack->AddWidget(WidgetClass);
}

void UPFPrimaryGameLayout::ClearLayer(FGameplayTag LayerID)
{
	if (UCommonActivatableWidgetStack* LayerStack = FindLayer(LayerID))
	{
		LayerStack->ClearWidgets();
	}
}

void UPFPrimaryGameLayout::ClearAllWidgets()
{
	for (const TPair<FGameplayTag, TObjectPtr<UCommonActivatableWidgetStack>>& Layer : m_Layers)
	{
		if (IsValid(Layer.Value.Get()))
		{
			Layer.Value->ClearWidgets();
		}
	}
}

UCommonActivatableWidget* UPFPrimaryGameLayout::GetActiveWidgetInLayer(FGameplayTag LayerID) const
{
	return GetActiveWidget(FindLayer(LayerID));
}

UCommonActivatableWidget* UPFPrimaryGameLayout::GetActiveGameWidget() const
{
	return GetActiveWidgetInLayer(SIHGameplayTags::UI_Layer_Game.GetTag());
}

UCommonActivatableWidget* UPFPrimaryGameLayout::GetActiveScreenWidget() const
{
	return GetActiveWidgetInLayer(SIHGameplayTags::UI_Layer_Screen.GetTag());
}

UCommonActivatableWidget* UPFPrimaryGameLayout::GetActiveModalWidget() const
{
	return GetActiveWidgetInLayer(SIHGameplayTags::UI_Layer_Modal.GetTag());
}

bool UPFPrimaryGameLayout::IsModalActive() const
{
	return IsValid(GetActiveModalWidget());
}

UCommonActivatableWidgetStack* UPFPrimaryGameLayout::FindLayer(FGameplayTag LayerID) const
{
	const TObjectPtr<UCommonActivatableWidgetStack>* LayerStack = m_Layers.Find(LayerID);
	if (LayerStack == nullptr || !IsValid(LayerStack->Get()))
	{
		PF_LOG(TEXT("UI Layer is not registered. LayerID=%s"), *LayerID.ToString());
		return nullptr;
	}

	return LayerStack->Get();
}
