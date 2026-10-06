#include "PFClueSubmissionWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/001_Data/000_Definitions/PFClueDefinition.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"
#include "Project_SIH/002_Systems/001_Investigation/Interfaces/PFMeetingCommandReceiver.h"
#include "Project_SIH/002_Systems/001_Investigation/PFInvestigationGameMode.h"
#include "Project_SIH/002_Systems/011_Dialogue/PFDialogueGraphExecutor.h"
#include "PFClueSubmissionEntryWidget.h"

void UPFClueSubmissionWidget::SetDialogueExecutor(
	UPFDialogueGraphExecutor* DialogueExecutor)
{
	m_DialogueExecutor = DialogueExecutor;
	if (IsActivated())
	{
		RefreshClueList();
	}
}

bool UPFClueSubmissionWidget::TryGetMeetingClueIDs(FGameplayTagContainer& OutClueIDs) const
{
	OutClueIDs.Reset();

	UWorld* World = GetWorld();
	AGameModeBase* GameMode = IsValid(World) ? World->GetAuthGameMode() : nullptr;
	const IPFMeetingCommandReceiver* Receiver = Cast<IPFMeetingCommandReceiver>(GameMode);
	if (Receiver == nullptr)
	{
		PF_LOG(TEXT("Meeting Command Receiver is not available."));
		return false;
	}

	if (!Receiver->TryGetAcquiredClueIDs(OutClueIDs))
	{
		PF_LOG(TEXT("Acquired Clue query was rejected."));
		return false;
	}

	PF_LOG(TEXT("[Clue Submission UI] MeetingClueCount=%d, MeetingClueIDs=%s"), OutClueIDs.Num(), *OutClueIDs.ToStringSimple());
	return true;
}

EPFMeetingSubmitResult UPFClueSubmissionWidget::TrySubmitClueWithMatchingClaim(FGameplayTag ClueID)
{
	UWorld* World = GetWorld();
	AGameModeBase* GameMode = IsValid(World) ? World->GetAuthGameMode() : nullptr;
	IPFMeetingCommandReceiver* Receiver = Cast<IPFMeetingCommandReceiver>(GameMode);
	if (Receiver == nullptr)
	{
		PF_LOG(TEXT("Meeting Command Receiver is not available."));
		return EPFMeetingSubmitResult::InactiveMeeting;
	}

	const UPFClueDefinition* ClueDefinition = UPFAssetManager::Get().GetClueDefinition(ClueID);
	if (!IsValid(ClueDefinition))
	{
		PF_LOG(TEXT("Clue definition is unavailable. ClueID=%s"), *ClueID.ToString());
		return EPFMeetingSubmitResult::NotReady;
	}
	if (!ClueDefinition->IsMeetingUsable())
	{
		PF_LOG(TEXT("Clue is not usable in Meeting. ClueID=%s"), *ClueID.ToString());
		return EPFMeetingSubmitResult::ClueNotMeetingUsable;
	}

	FGameplayTagContainer MeetingClueIDs;
	if (!Receiver->TryGetAcquiredClueIDs(MeetingClueIDs))
	{
		PF_LOG(TEXT("Acquired Clue query was rejected."));
		return EPFMeetingSubmitResult::InactiveMeeting;
	}
	if (!MeetingClueIDs.HasTagExact(ClueID))
	{
		PF_LOG(TEXT("Clue is not available for submission. ClueID=%s"), *ClueID.ToString());
		return EPFMeetingSubmitResult::ClueNotOwned;
	}

	const FGameplayTag& ClaimID = ClueDefinition->GetClaimID();
	const EPFMeetingClaimSelectResult ClaimResult = Receiver->TrySelectClaim(ClaimID);
	if (ClaimResult != EPFMeetingClaimSelectResult::Selected)
	{
		PF_LOG(TEXT("Matching Claim selection was rejected. ClaimID=%s, Result=%d"), *ClaimID.ToString(), static_cast<uint8>(ClaimResult));
		return ClaimResult == EPFMeetingClaimSelectResult::InactiveMeeting
			? EPFMeetingSubmitResult::InactiveMeeting
			: EPFMeetingSubmitResult::InvalidStep;
	}

	const EPFMeetingSubmitResult SubmitResult = Receiver->TrySubmitClue(ClueID);
	PF_LOG(TEXT("[Clue Submission UI] ClaimID=%s, ClueID=%s, Result=%d"), *ClaimID.ToString(), *ClueID.ToString(), static_cast<uint8>(SubmitResult));
	return SubmitResult;
}

bool UPFClueSubmissionWidget::TrySubmitClueForPendingDialogue(FGameplayTag ClueID)
{
	if (!IsValid(m_DialogueExecutor)
		|| m_DialogueExecutor->GetExecutionState()
			!= EPFDialogueExecutionState::WaitingForExternalResponse)
	{
		return false;
	}

	const FPFDialogueExternalSelectionRequest Request =
		m_DialogueExecutor->GetPendingExternalRequest();
	if (Request.RequestType != EPFDialogueExternalRequestType::Evidence)
	{
		return false;
	}

	UWorld* World = GetWorld();
	APFInvestigationGameMode* GameMode = IsValid(World)
		? Cast<APFInvestigationGameMode>(World->GetAuthGameMode())
		: nullptr;
	return IsValid(GameMode)
		&& GameMode->SubmitDialogueEvidenceSelection(
			m_DialogueExecutor,
			Request.RequestID,
			ClueID);
}

void UPFClueSubmissionWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	RefreshClueList();
}

void UPFClueSubmissionWidget::RefreshClueList()
{
	if (!IsValid(VerticalBox_ClueList))
	{
		PF_LOG(TEXT("Clue button VerticalBox is not bound."));
		return;
	}
	if (!m_ClueEntryClass)
	{
		PF_LOG(TEXT("Clue Entry class is not configured."));
		return;
	}

	VerticalBox_ClueList->ClearChildren();

	FGameplayTagContainer MeetingClueIDs;
	if (!TryGetMeetingClueIDs(MeetingClueIDs))
	{
		return;
	}

	for (const FGameplayTag& ClueID : MeetingClueIDs)
	{
		UPFClueSubmissionEntryWidget* ClueEntry = CreateWidget<UPFClueSubmissionEntryWidget>(GetOwningPlayer(), m_ClueEntryClass);
		if (!IsValid(ClueEntry))
		{
			PF_LOG(TEXT("Failed to create Clue Entry. ClueID=%s"), *ClueID.ToString());
			continue;
		}

		ClueEntry->SetClueID(ClueID);
		ClueEntry->OnClueClicked().AddUObject(this, &UPFClueSubmissionWidget::HandleClueClicked);
		VerticalBox_ClueList->AddChild(ClueEntry);
	}
}

void UPFClueSubmissionWidget::HandleClueClicked(const FGameplayTag& ClueID)
{
	if (IsValid(m_DialogueExecutor))
	{
		if (TrySubmitClueForPendingDialogue(ClueID))
		{
			DeactivateWidget();
		}
		return;
	}

	const EPFMeetingSubmitResult Result = TrySubmitClueWithMatchingClaim(ClueID);
	if (Result == EPFMeetingSubmitResult::Correct
		|| Result == EPFMeetingSubmitResult::Incorrect)
	{
		const FString ResultString = StaticEnum<EPFMeetingSubmitResult>()->GetNameStringByValue(static_cast<int64>(Result));

		PF_LOG(TEXT("Clue Submit : %s"), *ResultString);
		DeactivateWidget();
	}
}
