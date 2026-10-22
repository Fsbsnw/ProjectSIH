#include "PFClueSubmissionWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/002_Systems/001_Investigation/Interfaces/PFMeetingCommandReceiver.h"
#include "PFClueSubmissionEntryWidget.h"

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

EPFMeetingClaimSelectResult UPFClueSubmissionWidget::TrySelectClaim(FGameplayTag ClaimID)
{
	UWorld* World = GetWorld();
	AGameModeBase* GameMode = IsValid(World) ? World->GetAuthGameMode() : nullptr;
	IPFMeetingCommandReceiver* Receiver = Cast<IPFMeetingCommandReceiver>(GameMode);
	if (Receiver == nullptr)
	{
		PF_LOG(TEXT("Meeting Command Receiver is not available."));
		return EPFMeetingClaimSelectResult::InactiveMeeting;
	}

	const EPFMeetingClaimSelectResult ClaimResult = Receiver->TrySelectClaim(ClaimID);
	PF_LOG(TEXT("[Clue Submission UI] ClaimID=%s, Result=%d"), *ClaimID.ToString(), static_cast<uint8>(ClaimResult));
	return ClaimResult;
}

EPFMeetingSubmitResult UPFClueSubmissionWidget::TrySubmitClue(FGameplayTag ClueID)
{
	UWorld* World = GetWorld();
	AGameModeBase* GameMode = IsValid(World) ? World->GetAuthGameMode() : nullptr;
	IPFMeetingCommandReceiver* Receiver = Cast<IPFMeetingCommandReceiver>(GameMode);
	if (Receiver == nullptr)
	{
		PF_LOG(TEXT("Meeting Command Receiver is not available."));
		return EPFMeetingSubmitResult::InactiveMeeting;
	}

	const EPFMeetingSubmitResult SubmitResult = Receiver->TrySubmitClue(ClueID);
	PF_LOG(TEXT("[Clue Submission UI] ClueID=%s, Result=%d"), *ClueID.ToString(), static_cast<uint8>(SubmitResult));
	return SubmitResult;
}

void UPFClueSubmissionWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	RefreshClaimList();
}

void UPFClueSubmissionWidget::RefreshClaimList()
{
	if (!IsValid(VerticalBox_ClueList))
	{
		PF_LOG(TEXT("Submission VerticalBox is not bound."));
		return;
	}
	if (!m_ClueEntryClass)
	{
		PF_LOG(TEXT("Submission Entry class is not configured."));
		return;
	}

	VerticalBox_ClueList->ClearChildren();
	const FGameplayTag TestClaimIDs[] =
	{
		SIHGameplayTags::ID_Claim_DialogueTest_A.GetTag(),
		SIHGameplayTags::ID_Claim_DialogueTest_B.GetTag(),
	};

	for (const FGameplayTag& ClaimID : TestClaimIDs)
	{
		UPFClueSubmissionEntryWidget* ClaimEntry = CreateWidget<UPFClueSubmissionEntryWidget>(GetOwningPlayer(), m_ClueEntryClass);
		if (!IsValid(ClaimEntry))
		{
			PF_LOG(TEXT("Failed to create Claim Entry. ClaimID=%s"), *ClaimID.ToString());
			continue;
		}

		ClaimEntry->SetEntryID(ClaimID);
		ClaimEntry->OnEntryClicked().AddUObject(this, &UPFClueSubmissionWidget::HandleClaimClicked);
		VerticalBox_ClueList->AddChild(ClaimEntry);
	}
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

		ClueEntry->SetEntryID(ClueID);
		ClueEntry->OnEntryClicked().AddUObject(this, &UPFClueSubmissionWidget::HandleClueClicked);
		VerticalBox_ClueList->AddChild(ClueEntry);
	}
}

void UPFClueSubmissionWidget::HandleClaimClicked(const FGameplayTag& ClaimID)
{
	if (TrySelectClaim(ClaimID) == EPFMeetingClaimSelectResult::Selected)
	{
		RefreshClueList();
	}
}

void UPFClueSubmissionWidget::HandleClueClicked(const FGameplayTag& ClueID)
{
	const EPFMeetingSubmitResult Result = TrySubmitClue(ClueID);
	if (Result == EPFMeetingSubmitResult::Correct
		|| Result == EPFMeetingSubmitResult::Incorrect)
	{
		const FString ResultString = StaticEnum<EPFMeetingSubmitResult>()->GetNameStringByValue(static_cast<int64>(Result));

		PF_LOG(TEXT("Clue Submit : %s"), *ResultString);
		DeactivateWidget();
	}
}
