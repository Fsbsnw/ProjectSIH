#include "PFClueInteractableActor.h"

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFClueTypes.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/001_Data/000_Definitions/PFDialogueDefinition.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"
#include "Project_SIH/002_Systems/001_Investigation/Interfaces/PFInvestigationCommandReceiver.h"
#include "Project_SIH/002_Systems/011_Dialogue/PFDialogueSubsystem.h"
#include "Project_SIH/003_UI/002_Subsystem/PFUIManagerSubsystem.h"

namespace
{
	const FName AcquireResult_Acquired(TEXT("Acquired"));
	const FName AcquireResult_AlreadyOwned(TEXT("AlreadyOwned"));
	const FName AcquireResult_Failed(TEXT("Failed"));

	const TCHAR* GetAcquireResultText(const EPFClueAcquireResult Result)
	{
		switch (Result)
		{
		case EPFClueAcquireResult::None:
			return TEXT("None");
		case EPFClueAcquireResult::Acquired:
			return TEXT("Acquired");
		case EPFClueAcquireResult::AlreadyOwned:
			return TEXT("AlreadyOwned");
		case EPFClueAcquireResult::InvalidClueID:
			return TEXT("InvalidClueID");
		case EPFClueAcquireResult::DefinitionUnavailable:
			return TEXT("DefinitionUnavailable");
		case EPFClueAcquireResult::InactiveCase:
			return TEXT("InactiveCase");
		default:
			return TEXT("Unknown");
		}
	}
}

bool APFClueInteractableActor::Interact(APlayerController* Interactor)
{
	if (!IsValid(Interactor))
	{
		PF_LOG(TEXT("Clue interaction request has no valid Interactor"));
		return false;
	}

	if (m_DialogueID.IsValid())
	{
		return TryStartDialogue(Interactor);
	}

	const EPFClueAcquireResult Result = TryAcquireClue();
	return Result == EPFClueAcquireResult::Acquired || Result == EPFClueAcquireResult::AlreadyOwned;
}

bool APFClueInteractableActor::TryStartDialogue(
	APlayerController* Interactor)
{
	UPFDialogueDefinition* DialogueDefinition =
		UPFAssetManager::Get().GetDialogueDefinition(m_DialogueID);
	if (!IsValid(DialogueDefinition))
	{
		PF_LOG(
			TEXT("Interaction Dialogue Definition is unavailable. DialogueID=%s"),
			*m_DialogueID.ToString());
		return false;
	}

	UWorld* World = GetWorld();
	UPFDialogueSubsystem* DialogueSubsystem = IsValid(World)
		? World->GetSubsystem<UPFDialogueSubsystem>()
		: nullptr;
	if (!IsValid(DialogueSubsystem))
	{
		PF_LOG(TEXT("Dialogue Subsystem is not available"));
		return false;
	}

	BindDialogueEvents(DialogueSubsystem);
	const EPFDialogueInputResult StartResult =
		DialogueSubsystem->StartDialogue(DialogueDefinition);
	if (StartResult != EPFDialogueInputResult::Accepted
		|| !DialogueSubsystem->IsDialogueActive())
	{
		PF_LOG(
			TEXT("Interaction Dialogue start was rejected. DialogueID=%s, Result=%d"),
			*m_DialogueID.ToString(),
			static_cast<uint8>(StartResult));
		UnbindDialogueEvents();
		return false;
	}

	ULocalPlayer* LocalPlayer = Interactor->GetLocalPlayer();
	UPFUIManagerSubsystem* UIManager = IsValid(LocalPlayer)
		? LocalPlayer->GetSubsystem<UPFUIManagerSubsystem>()
		: nullptr;
	if (!IsValid(UIManager)
		|| !UIManager->ShowWidget(SIHGameplayTags::UI_Screen_Dialogue.GetTag()))
	{
		DialogueSubsystem->CancelDialogue();
		PF_LOG(TEXT("Interaction Dialogue screen could not be shown"));
		return false;
	}

	return true;
}

EPFClueAcquireResult APFClueInteractableActor::TryAcquireClue() const
{
	if (!m_ClueID.IsValid())
	{
		PF_LOG(TEXT("Clue interaction request is invalid. ClueID=%s"), *m_ClueID.ToString());
		return EPFClueAcquireResult::InvalidClueID;
	}

	AGameModeBase* GameMode = GetWorld() != nullptr ? GetWorld()->GetAuthGameMode() : nullptr;
	IPFInvestigationCommandReceiver* CommandReceiver = Cast<IPFInvestigationCommandReceiver>(GameMode);
	if (CommandReceiver == nullptr)
	{
		PF_LOG(TEXT("Current GameMode does not provide Investigation commands"));
		return EPFClueAcquireResult::InactiveCase;
	}

	const EPFClueAcquireResult Result = CommandReceiver->TryAcquireClue(m_ClueID);
	PF_LOG(TEXT("Clue interaction completed. ClueID=%s, Result=%s"), *m_ClueID.ToString(), GetAcquireResultText(Result));
	return Result;
}

void APFClueInteractableActor::BindDialogueEvents(UPFDialogueSubsystem* DialogueSubsystem)
{
	UnbindDialogueEvents();
	m_DialogueSubsystem = DialogueSubsystem;
	if (!IsValid(m_DialogueSubsystem))
	{
		return;
	}

	m_DialogueSubsystem->GetActionRequestedDelegate().AddUniqueDynamic(this, &APFClueInteractableActor::HandleDialogueActionRequested);
	m_DialogueSubsystem->GetCompletedDelegate().AddUniqueDynamic(this, &APFClueInteractableActor::HandleDialogueFinished);
	m_DialogueSubsystem->GetFailedDelegate().AddUniqueDynamic(this, &APFClueInteractableActor::HandleDialogueFinished);
	m_DialogueSubsystem->GetCancelledDelegate().AddUniqueDynamic(this, &APFClueInteractableActor::HandleDialogueFinished);
}

void APFClueInteractableActor::UnbindDialogueEvents()
{
	if (!IsValid(m_DialogueSubsystem))
	{
		m_DialogueSubsystem = nullptr;
		return;
	}

	m_DialogueSubsystem->GetActionRequestedDelegate().RemoveDynamic(this, &APFClueInteractableActor::HandleDialogueActionRequested);
	m_DialogueSubsystem->GetCompletedDelegate().RemoveDynamic(this, &APFClueInteractableActor::HandleDialogueFinished);
	m_DialogueSubsystem->GetFailedDelegate().RemoveDynamic(this, &APFClueInteractableActor::HandleDialogueFinished);
	m_DialogueSubsystem->GetCancelledDelegate().RemoveDynamic(this, &APFClueInteractableActor::HandleDialogueFinished);
	m_DialogueSubsystem = nullptr;
}

void APFClueInteractableActor::HandleDialogueActionRequested(const FPFDialogueActionPresentation& Presentation)
{
	if (!Presentation.m_Action.m_ActionID.MatchesTagExact(SIHGameplayTags::Action_Dialogue_AcquireClue.GetTag()))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		PF_LOG(TEXT("Acquire Clue Dialogue Action has no valid World"));
		return;
	}

	const FName ActionNodeID = Presentation.m_NodeID;
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, ActionNodeID]()
	{
		ExecuteAcquireClueAction(ActionNodeID);
	}));
}

void APFClueInteractableActor::ExecuteAcquireClueAction(const FName ActionNodeID)
{
	if (!IsValid(m_DialogueSubsystem))
	{
		return;
	}

	const EPFClueAcquireResult AcquireResult = TryAcquireClue();
	FName DialogueResultID = AcquireResult_Failed;
	if (AcquireResult == EPFClueAcquireResult::Acquired)
	{
		DialogueResultID = AcquireResult_Acquired;
	}
	else if (AcquireResult == EPFClueAcquireResult::AlreadyOwned)
	{
		DialogueResultID = AcquireResult_AlreadyOwned;
	}

	const EPFDialogueInputResult CompleteResult = m_DialogueSubsystem->CompleteAction(ActionNodeID, DialogueResultID);
	if (CompleteResult != EPFDialogueInputResult::Accepted)
	{
		PF_LOG(TEXT("Acquire Clue Dialogue Action completion was rejected. NodeID=%s, ResultID=%s, Result=%d"), *ActionNodeID.ToString(), *DialogueResultID.ToString(), static_cast<uint8>(CompleteResult));
	}
}

void APFClueInteractableActor::HandleDialogueFinished()
{
	UnbindDialogueEvents();
}

void APFClueInteractableActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindDialogueEvents();
	Super::EndPlay(EndPlayReason);
}
