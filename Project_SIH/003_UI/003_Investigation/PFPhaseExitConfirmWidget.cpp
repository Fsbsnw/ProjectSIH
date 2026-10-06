#include "PFPhaseExitConfirmWidget.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/000_GameFlow/PFGameFlowSubsystem.h"
#include "Project_SIH/002_Systems/001_Investigation/Interfaces/PFInvestigationCommandReceiver.h"
#include "Project_SIH/002_Systems/001_Investigation/Interfaces/PFMeetingCommandReceiver.h"

void UPFPhaseExitConfirmWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (IsValid(Button_Confirm))
	{
		Button_Confirm->OnClicked().AddUObject(
			this,
			&UPFPhaseExitConfirmWidget::HandleConfirmClicked);
	}
	if (IsValid(Button_Cancel))
	{
		Button_Cancel->OnClicked().AddUObject(
			this,
			&UPFPhaseExitConfirmWidget::HandleCancelClicked);
	}
}

void UPFPhaseExitConfirmWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	m_bResponseSent = false;
	ResetPendingPhaseExit();

	if (!PreparePhaseExit())
	{
		m_bResponseSent = true;
		DeactivateWidget();
	}
}

void UPFPhaseExitConfirmWidget::NativeOnDeactivated()
{
	ResetPendingPhaseExit();
	Super::NativeOnDeactivated();
}

UWidget* UPFPhaseExitConfirmWidget::NativeGetDesiredFocusTarget() const
{
	return IsValid(Button_Confirm)
		? Button_Confirm
		: Super::NativeGetDesiredFocusTarget();
}

bool UPFPhaseExitConfirmWidget::NativeOnHandleBackAction()
{
	HandleCancelClicked();
	return true;
}

bool UPFPhaseExitConfirmWidget::PreparePhaseExit()
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UPFGameFlowSubsystem* GameFlow = IsValid(GameInstance)
		? GameInstance->GetSubsystem<UPFGameFlowSubsystem>()
		: nullptr;
	if (!IsValid(GameFlow))
	{
		PF_LOG(TEXT("GameFlowSubsystem is not available."));
		return false;
	}

	const FPFActiveCaseState& ActiveCaseState = GameFlow->GetActiveCaseState();
	const FText* Prompt = nullptr;
	if (ActiveCaseState.IsActiveInPhase(EPFCasePhase::Investigation))
	{
		Prompt = &m_InvestigationExitPrompt;
	}
	else if (ActiveCaseState.IsActiveInPhase(EPFCasePhase::Meeting))
	{
		Prompt = &m_MeetingExitPrompt;
	}
	else
	{
		PF_LOG(
			TEXT("Phase exit confirmation is unavailable. Phase=%d"),
			static_cast<uint8>(ActiveCaseState.m_CasePhase));
		return false;
	}

	m_PendingCaseID = ActiveCaseState.m_CaseID;
	m_PendingPhase = ActiveCaseState.m_CasePhase;
	if (IsValid(Text_Prompt))
	{
		Text_Prompt->SetText(*Prompt);
	}

	return true;
}

bool UPFPhaseExitConfirmWidget::TryCompletePhaseExit()
{
	UGameInstance* GameInstance = GetGameInstance();
	UPFGameFlowSubsystem* GameFlow = IsValid(GameInstance)
		? GameInstance->GetSubsystem<UPFGameFlowSubsystem>()
		: nullptr;
	if (!IsValid(GameFlow)
		|| !GameFlow->GetActiveCaseState().MatchesCaseAndPhase(
			m_PendingCaseID,
			m_PendingPhase))
	{
		PF_LOG(TEXT("Case state changed before the Phase exit was confirmed."));
		return false;
	}

	UWorld* World = GetWorld();
	AGameModeBase* GameMode = IsValid(World) ? World->GetAuthGameMode() : nullptr;
	bool bCompleted = false;
	if (m_PendingPhase == EPFCasePhase::Investigation)
	{
		IPFInvestigationCommandReceiver* Receiver =
			Cast<IPFInvestigationCommandReceiver>(GameMode);
		bCompleted = Receiver && Receiver->TryCompleteInvestigation(m_PendingCaseID);
	}
	else if (m_PendingPhase == EPFCasePhase::Meeting)
	{
		IPFMeetingCommandReceiver* Receiver = Cast<IPFMeetingCommandReceiver>(GameMode);
		bCompleted = Receiver && Receiver->TryCompleteMeeting(m_PendingCaseID);
	}

	if (!bCompleted)
	{
		PF_LOG(
			TEXT("Phase exit command was rejected. CaseID=%s, Phase=%d"),
			*m_PendingCaseID.ToString(),
			static_cast<uint8>(m_PendingPhase));
		return false;
	}

	LogCompletionResult();
	return true;
}

void UPFPhaseExitConfirmWidget::LogCompletionResult() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UPFGameFlowSubsystem* GameFlow = IsValid(GameInstance)
		? GameInstance->GetSubsystem<UPFGameFlowSubsystem>()
		: nullptr;
	if (!IsValid(GameFlow))
	{
		return;
	}

	const FPFActiveCaseState& UpdatedCaseState = GameFlow->GetActiveCaseState();
	if (m_PendingPhase == EPFCasePhase::Investigation)
	{
		const FGameplayTagContainer& AcquiredClueIDs =
			UpdatedCaseState.m_InvestigationResult.m_AcquiredClueIDs;
		const FString AcquiredClueLog = AcquiredClueIDs.IsEmpty()
			? TEXT("None")
			: AcquiredClueIDs.ToStringSimple();
		PF_LOG(
			TEXT("[Investigation Complete] CaseID=%s, AcquiredClueCount=%d, AcquiredClueIDs=%s"),
			*m_PendingCaseID.ToString(),
			AcquiredClueIDs.Num(),
			*AcquiredClueLog);
	}
	else if (m_PendingPhase == EPFCasePhase::Meeting)
	{
		const TArray<FGameplayTag>& RevealedWeaknessIDs =
			UpdatedCaseState.m_MeetingResult.m_RevealedWeaknessIDs;
		const FString RevealedWeaknessLog = RevealedWeaknessIDs.IsEmpty()
			? TEXT("None")
			: FString::JoinBy(
				RevealedWeaknessIDs,
				TEXT(", "),
				[](const FGameplayTag& WeaknessID)
				{
					return WeaknessID.ToString();
				});
		PF_LOG(
			TEXT("[Meeting Complete] CaseID=%s, RevealedWeaknessCount=%d, RevealedWeaknessIDs=%s"),
			*m_PendingCaseID.ToString(),
			RevealedWeaknessIDs.Num(),
			*RevealedWeaknessLog);
	}
}

void UPFPhaseExitConfirmWidget::ResetPendingPhaseExit()
{
	m_PendingCaseID = FGameplayTag{};
	m_PendingPhase = EPFCasePhase::None;
}

void UPFPhaseExitConfirmWidget::HandleConfirmClicked()
{
	if (m_bResponseSent)
	{
		return;
	}

	m_bResponseSent = true;
	TryCompletePhaseExit();
	if (IsActivated())
	{
		DeactivateWidget();
	}
}

void UPFPhaseExitConfirmWidget::HandleCancelClicked()
{
	if (m_bResponseSent)
	{
		return;
	}

	m_bResponseSent = true;
	if (IsActivated())
	{
		DeactivateWidget();
	}
}
