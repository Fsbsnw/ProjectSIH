#include "PFBattleCharacterStatusWidget.h"

#include "AbilitySystemComponent.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleMessages.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/003_Combat/001_Characters/PFBattleCharacterBase.h"
#include "Project_SIH/002_Systems/003_Combat/002_Attributes/PFBattleAttributeSet.h"
#include "Project_SIH/SIHGameplayTags.h"

void UPFBattleCharacterStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetActionOwnerHighlighted(false);

	FGameplayMessageListenerParams<FPFBattleActionStateMessage> Params;
	Params.MatchType = EGameplayMessageMatch::PartialMatch;
	Params.SetMessageReceivedCallback(
		this,
		&ThisClass::HandleActionStateChanged);

	m_ActionStateMessageHandle =
		UGameplayMessageSubsystem::Get(this)
			.RegisterListener<FPFBattleActionStateMessage>(
				SIHGameplayTags::Message_Battle_ActionState.GetTag(),
				Params);
}

void UPFBattleCharacterStatusWidget::SetCharacter(
	APFBattleCharacterBase* Character,
	const FText& DisplayName)
{
	UnbindCharacter();

	if (!IsValid(Character))
	{
		PF_LOG(TEXT("Battle party member character is invalid."));
		return;
	}

	UAbilitySystemComponent* ASC =
		Character->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		PF_LOG(TEXT("Battle party member ASC is invalid."));
		return;
	}

	bool bFoundHP = false;
	bool bFoundMaxHP = false;
	m_CurrentHP = ASC->GetGameplayAttributeValue(
		UPFBattleAttributeSet::GetHPAttribute(),
		bFoundHP);
	m_CurrentMaxHP = ASC->GetGameplayAttributeValue(
		UPFBattleAttributeSet::GetMaxHPAttribute(),
		bFoundMaxHP);

	if (!bFoundHP || !bFoundMaxHP)
	{
		PF_LOG(TEXT("Battle party member HP attributes are missing."));
		return;
	}

	Text_CharacterName->SetText(DisplayName);
	m_ObservedCharacter = Character;
	m_ObservedASC = ASC;

	m_HPChangedHandle = ASC
		->GetGameplayAttributeValueChangeDelegate(
			UPFBattleAttributeSet::GetHPAttribute())
		.AddUObject(
			this,
			&ThisClass::HandleHPChanged);

	m_MaxHPChangedHandle = ASC
		->GetGameplayAttributeValueChangeDelegate(
			UPFBattleAttributeSet::GetMaxHPAttribute())
		.AddUObject(
			this,
			&ThisClass::HandleMaxHPChanged);

	RefreshHP();
}

void UPFBattleCharacterStatusWidget::NativeDestruct()
{
	m_ActionStateMessageHandle.Unregister();
	UnbindCharacter();
	Super::NativeDestruct();
}

void UPFBattleCharacterStatusWidget::UnbindCharacter()
{
	if (UAbilitySystemComponent* ASC = m_ObservedASC.Get())
	{
		if (m_HPChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(
				UPFBattleAttributeSet::GetHPAttribute())
				.Remove(m_HPChangedHandle);
		}

		if (m_MaxHPChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(
				UPFBattleAttributeSet::GetMaxHPAttribute())
				.Remove(m_MaxHPChangedHandle);
		}
	}

	m_HPChangedHandle.Reset();
	m_MaxHPChangedHandle.Reset();
	m_ObservedASC.Reset();
	m_CurrentHP = 0.0f;
	m_CurrentMaxHP = 0.0f;
	m_ObservedCharacter.Reset();
	SetActionOwnerHighlighted(false);
}

void UPFBattleCharacterStatusWidget::HandleActionStateChanged(
	FGameplayTag Channel,
	const FPFBattleActionStateMessage& Message)
{
	SetActionOwnerHighlighted(
		m_ObservedCharacter.IsValid()
		&& m_ObservedCharacter.Get() == Message.m_ActionOwner.Get());
}

void UPFBattleCharacterStatusWidget::SetActionOwnerHighlighted(
	const bool bHighlighted)
{
	Border_CurrentActionOwner->SetBrushColor(
		bHighlighted
			? FLinearColor(1.0f, 0.8f, 0.0f, 1.0f)
			: FLinearColor::Transparent);
}

void UPFBattleCharacterStatusWidget::HandleHPChanged(
	const FOnAttributeChangeData& ChangeData)
{
	m_CurrentHP = ChangeData.NewValue;
	RefreshHP();
}

void UPFBattleCharacterStatusWidget::HandleMaxHPChanged(
	const FOnAttributeChangeData& ChangeData)
{
	m_CurrentMaxHP = ChangeData.NewValue;
	RefreshHP();
}

void UPFBattleCharacterStatusWidget::RefreshHP()
{
	ProgressBar_HP->SetPercent(
		m_CurrentMaxHP > 0.0f
			? FMath::Clamp(
				m_CurrentHP / m_CurrentMaxHP,
				0.0f,
				1.0f)
			: 0.0f);

	Text_HP->SetText(FText::Format(
		NSLOCTEXT(
			"PFBattleCharacterStatusWidget",
			"HPFormat",
			"{0} / {1}"),
		FText::AsNumber(FMath::CeilToInt(m_CurrentHP)),
		FText::AsNumber(FMath::CeilToInt(m_CurrentMaxHP))));
}
