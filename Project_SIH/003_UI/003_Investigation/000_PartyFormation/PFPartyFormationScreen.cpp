#include "PFPartyFormationScreen.h"

#include "Blueprint/UserWidget.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/ScrollBox.h"
#include "Engine/GameInstance.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/000_Core/001_Contracts/002_Party/PFPartyTypes.h"
#include "Project_SIH/001_Data/000_Definitions/PFCharacterDefinition.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"
#include "Project_SIH/002_Systems/000_GameFlow/PFGameFlowSubsystem.h"
#include "Project_SIH/002_Systems/004_CharacterState/PFCharacterStateSubsystem.h"
#include "PFPartyCandidateEntryWidget.h"
#include "PFPartySlotWidget.h"

void UPFPartyFormationScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!IsValid(Button_Deploy))
	{
		PF_LOG(TEXT("Party Deploy button is not bound."));
		return;
	}

	Button_Deploy->OnClicked().AddUObject(
		this, &UPFPartyFormationScreen::HandleDeployClicked);
}

void UPFPartyFormationScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	m_SelectedCharacterIDs.Reset();
	if (IsValid(HorizontalBox_Slots))
	{
		for (int32 SlotIndex = 0;
			SlotIndex < FPFPartyData::RequiredMemberCount;
			++SlotIndex)
		{
			if (SlotIndex >= HorizontalBox_Slots->GetChildrenCount())
			{
				PF_LOG(TEXT("Party slot widget is missing. SlotIndex=%d"), SlotIndex);
				continue;
			}

			UPFPartySlotWidget* PartySlot = Cast<UPFPartySlotWidget>(
				HorizontalBox_Slots->GetChildAt(SlotIndex));
			if (!IsValid(PartySlot))
			{
				PF_LOG(TEXT("Party slot widget is unavailable. SlotIndex=%d"), SlotIndex);
				continue;
			}

			PartySlot->SetSlot(
				SlotIndex, FGameplayTag(), FText::GetEmpty(), nullptr);
		}
	}
	else
	{
		PF_LOG(TEXT("Party slot HorizontalBox is not bound."));
	}

	RefreshCandidateList();
	RefreshSelectedCount();
}

bool UPFPartyFormationScreen::TryGetCandidateCharacterIDs(
	TArray<FGameplayTag>& OutCharacterIDs) const
{
	OutCharacterIDs.Reset();

	UGameInstance* GameInstance = GetGameInstance();
	const UPFGameFlowSubsystem* GameFlow = IsValid(GameInstance)
		? GameInstance->GetSubsystem<UPFGameFlowSubsystem>()
		: nullptr;
	const UPFCharacterStateSubsystem* CharacterStates = IsValid(GameInstance)
		? GameInstance->GetSubsystem<UPFCharacterStateSubsystem>()
		: nullptr;

	if (!IsValid(GameFlow)
		|| !GameFlow->GetActiveCaseState().IsActiveInPhase(
			EPFCasePhase::PartyFormation)
		|| !IsValid(CharacterStates))
	{
		PF_LOG(TEXT("Party candidates are unavailable in the current GameFlow state."));
		return false;
	}

	OutCharacterIDs = CharacterStates->GetCharacterIDs();
	return true;
}

void UPFPartyFormationScreen::RefreshCandidateList()
{
	if (!IsValid(ScrollBox_Candidates))
	{
		PF_LOG(TEXT("Party candidate ScrollBox is not bound."));
		return;
	}
	if (!m_CandidateEntryClass)
	{
		PF_LOG(TEXT("Party candidate Entry class is not configured."));
		return;
	}

	ScrollBox_Candidates->ClearChildren();

	TArray<FGameplayTag> CharacterIDs;
	if (!TryGetCandidateCharacterIDs(CharacterIDs))
	{
		PF_LOG(TEXT("Failed to retrieve party candidate CharacterIDs."));
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const UPFCharacterStateSubsystem* CharacterStates =
		GameInstance->GetSubsystem<UPFCharacterStateSubsystem>();
	UPFAssetManager& AssetManager = UPFAssetManager::Get();

	for (const FGameplayTag& CharacterID : CharacterIDs)
	{
		FPFCharacterStateData CharacterState;
		if (!CharacterStates->TryGetCharacterState(CharacterID, CharacterState))
		{
			PF_LOG(TEXT("Party candidate state is unavailable. CharacterID=%s"),
				*CharacterID.ToString());
			continue;
		}

		const UPFCharacterDefinition* Definition =
			AssetManager.GetCharacterDefinition(CharacterID);
		if (!IsValid(Definition))
		{
			PF_LOG(TEXT("Party candidate Definition is unavailable. CharacterID=%s"),
				*CharacterID.ToString());
			continue;
		}

		UPFPartyCandidateEntryWidget* Entry =
			CreateWidget<UPFPartyCandidateEntryWidget>(
				GetOwningPlayer(), m_CandidateEntryClass);
		if (!IsValid(Entry))
		{
			PF_LOG(TEXT("Failed to create Party candidate Entry. CharacterID=%s"),
				*CharacterID.ToString());
			continue;
		}

		Entry->SetCandidate(
			CharacterID,
			Definition->GetDisplayName(),
			CharacterState.m_ProgressData.m_Level,
			Definition->GetIcon().LoadSynchronous());

		Entry->OnCandidateClicked().AddUObject(
			this, &UPFPartyFormationScreen::HandleCandidateClicked);
		ScrollBox_Candidates->AddChild(Entry);
	}
}

void UPFPartyFormationScreen::HandleCandidateClicked(
	const FGameplayTag& CharacterID)
{
	if (m_SelectedCharacterIDs.Num() >= FPFPartyData::RequiredMemberCount)
	{
		PF_LOG(TEXT("All party slots are filled. CharacterID=%s"),
			*CharacterID.ToString());
		return;
	}
	if (m_SelectedCharacterIDs.Contains(CharacterID))
	{
		PF_LOG(TEXT("Party candidate is already selected. CharacterID=%s"),
			*CharacterID.ToString());
		return;
	}
	if (!IsValid(HorizontalBox_Slots))
	{
		PF_LOG(TEXT("Party slot HorizontalBox is not bound."));
		return;
	}

	const int32 SlotIndex = m_SelectedCharacterIDs.Num();
	if (SlotIndex >= HorizontalBox_Slots->GetChildrenCount())
	{
		PF_LOG(TEXT("Party slot widget is missing. SlotIndex=%d"), SlotIndex);
		return;
	}

	UPFPartySlotWidget* PartySlot = Cast<UPFPartySlotWidget>(
		HorizontalBox_Slots->GetChildAt(SlotIndex));
	const UPFCharacterDefinition* Definition =
		UPFAssetManager::Get().GetCharacterDefinition(CharacterID);
	if (!IsValid(PartySlot) || !IsValid(Definition))
	{
		PF_LOG(TEXT("Party slot or candidate Definition is unavailable. SlotIndex=%d, CharacterID=%s"),
			SlotIndex, *CharacterID.ToString());
		return;
	}

	PartySlot->SetSlot(
		SlotIndex,
		CharacterID,
		Definition->GetDisplayName(),
		Definition->GetIcon().LoadSynchronous());
	m_SelectedCharacterIDs.Add(CharacterID);
	RefreshSelectedCount();
}

void UPFPartyFormationScreen::RefreshSelectedCount()
{
	if (!IsValid(Text_SelectedCount) || !IsValid(Button_Deploy))
	{
		PF_LOG(TEXT("Party count text or Deploy button is not bound."));
		return;
	}

	Text_SelectedCount->SetText(FText::Format(
		NSLOCTEXT("PFPartyFormationScreen", "SelectedCount", "{0} / {1}"),
		FText::AsNumber(m_SelectedCharacterIDs.Num()),
		FText::AsNumber(FPFPartyData::RequiredMemberCount)));
	Button_Deploy->SetIsEnabled(
		m_SelectedCharacterIDs.Num() == FPFPartyData::RequiredMemberCount);
}

void UPFPartyFormationScreen::HandleDeployClicked()
{
	if (m_SelectedCharacterIDs.Num() != FPFPartyData::RequiredMemberCount)
	{
		PF_LOG(TEXT("Party requires exactly four characters. Selected=%d"),
			m_SelectedCharacterIDs.Num());
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UPFGameFlowSubsystem* GameFlow = IsValid(GameInstance)
		? GameInstance->GetSubsystem<UPFGameFlowSubsystem>()
		: nullptr;
	if (!IsValid(GameFlow))
	{
		PF_LOG(TEXT("GameFlow is unavailable for Party confirmation."));
		return;
	}

	FPFPartyData Party;
	for (const FGameplayTag& CharacterID : m_SelectedCharacterIDs)
	{
		Party.m_CharacterIDs.AddTag(CharacterID);
	}

	if (!GameFlow->TryConfirmParty(
		GameFlow->GetActiveCaseState().m_CaseID, Party))
	{
		PF_LOG(TEXT("Party confirmation was rejected by GameFlow."));
	}
}
