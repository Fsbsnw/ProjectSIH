#include "PFUIConfig.h"

const TSoftClassPtr<UPFGameUIRootBase>& UPFUIConfig::GetHUDLayOutClass() const
{
	return m_HUDLayOutClass;
}

const FPFUIWidgetDefinition* UPFUIConfig::FindGameWidgetDefinition(
	const FGameplayTag& WidgetID) const
{
	return m_GameWidgetDefinitions.Find(WidgetID);
}

const FPFUIWidgetDefinition* UPFUIConfig::FindScreenWidgetDefinition(
	const FGameplayTag& WidgetID) const
{
	return m_ScreenWidgetDefinitions.Find(WidgetID);
}

const FPFUIWidgetDefinition* UPFUIConfig::FindModalWidgetDefinition(
	const FGameplayTag& WidgetID) const
{
	return m_ModalWidgetDefinitions.Find(WidgetID);
}

void UPFUIConfig::GetWidgetDefinitions(
	TMap<FGameplayTag, FPFUIWidgetDefinition>& OutDefinitions) const
{
	OutDefinitions.Reset();
	OutDefinitions.Append(m_GameWidgetDefinitions);
	OutDefinitions.Append(m_ScreenWidgetDefinitions);
	OutDefinitions.Append(m_ModalWidgetDefinitions);
}
