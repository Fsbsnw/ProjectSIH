#include "PFBattleUIRoot.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Project_SIH/000_Core/001_Contracts/002_Party/PFPartyTypes.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleMessages.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/001_Data/000_Definitions/PFCharacterDefinition.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"
#include "Project_SIH/002_Systems/003_Combat/001_Characters/PFBattleCharacterBase.h"
#include "Project_SIH/003_UI/002_Subsystem/PFUIManagerSubsystem.h"
#include "Project_SIH/003_UI/004_Battle/PFBattleCharacterStatusWidget.h"
#include "Project_SIH/003_UI/004_Battle/PFBattleResultScreen.h"
#include "Project_SIH/SIHGameplayTags.h"

void UPFBattleUIRoot::NativeConstruct()
{
	Super::NativeConstruct();

	m_ParticipantsMessageHandle =
		UGameplayMessageSubsystem::Get(this)
			.RegisterListener<FPFBattleParticipantsInitializedMessage>(
				SIHGameplayTags::Message_Battle_Participants_Initialized.GetTag(),
				this,
				&ThisClass::HandleParticipantsInitialized);

	m_BattleCompletedHandle =
		UGameplayMessageSubsystem::Get(this)
			.RegisterListener<FPFBattleResult>(
				SIHGameplayTags::Message_Flow_Battle_Completed.GetTag(),
				this,
				&ThisClass::HandleBattleCompleted);
}

void UPFBattleUIRoot::NativeDestruct()
{
	m_BattleCompletedHandle.Unregister();
	m_ParticipantsMessageHandle.Unregister();
	Super::NativeDestruct();
}

void UPFBattleUIRoot::HandleParticipantsInitialized(
	FGameplayTag Channel,
	const FPFBattleParticipantsInitializedMessage& Message)
{
	if (Message.m_Allies.Num() != FPFPartyData::RequiredMemberCount)
	{
		PF_LOG(TEXT("Battle participant count is invalid. Allies=%d"),
			Message.m_Allies.Num());
		return;
	}

	UPFBattleCharacterStatusWidget* PartyStatusWidgets[] = {
		Status_Party01.Get(),
		Status_Party02.Get(),
		Status_Party03.Get(),
		Status_Party04.Get()
	};

	for (int32 Index = 0; Index < FPFPartyData::RequiredMemberCount; ++Index)
	{
		SetStatusWidget(PartyStatusWidgets[Index], Message.m_Allies[Index]);
	}

	SetStatusWidget(Status_Boss.Get(), Message.m_Boss);
}

void UPFBattleUIRoot::HandleBattleCompleted(
	FGameplayTag,
	const FPFBattleResult& Result)
{
	APlayerController* PlayerController = GetOwningPlayer();
	ULocalPlayer* LocalPlayer = IsValid(PlayerController)
		? PlayerController->GetLocalPlayer()
		: nullptr;
	UPFUIManagerSubsystem* UIManager = IsValid(LocalPlayer)
		? LocalPlayer->GetSubsystem<UPFUIManagerSubsystem>()
		: nullptr;
	if (!IsValid(UIManager))
	{
		PF_LOG(TEXT("UIManagerSubsystem is not available."));
		return;
	}

	UCommonActivatableWidget* ShownWidget =
		UIManager->ShowWidget(
			SIHGameplayTags::UI_Screen_BattleResult.GetTag());
	if (!IsValid(ShownWidget))
	{
		return;
	}

	UPFBattleResultScreen* ResultScreen =
		Cast<UPFBattleResultScreen>(ShownWidget);
	if (!IsValid(ResultScreen))
	{
		PF_LOG(TEXT("Battle result screen class is configured incorrectly."));
		ShownWidget->DeactivateWidget();
		return;
	}

	ResultScreen->SetResult(Result);
}

void UPFBattleUIRoot::SetStatusWidget(
	UPFBattleCharacterStatusWidget* StatusWidget,
	const FPFBattleParticipantEntry& Entry)
{
	if (!IsValid(StatusWidget))
	{
		PF_LOG(TEXT("Battle character status widget is missing. CharacterID=%s"),
			*Entry.m_CharacterID.ToString());
		return;
	}

	const UPFCharacterDefinition* Definition =
		UPFAssetManager::Get().GetCharacterDefinition(Entry.m_CharacterID);
	if (!IsValid(Definition))
	{
		PF_LOG(TEXT("Battle character Definition is unavailable. CharacterID=%s"),
			*Entry.m_CharacterID.ToString());
		return;
	}

	StatusWidget->SetCharacter(
		Entry.m_Character.Get(),
		Definition->GetDisplayName());
}
