#include "PFClueInteractableActor.h"

#include "GameFramework/GameModeBase.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFClueTypes.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/001_Investigation/Interfaces/PFInvestigationCommandReceiver.h"

namespace
{
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
	if (!IsValid(Interactor) || !m_ClueID.IsValid())
	{
		PF_LOG(TEXT("Clue interaction request is invalid. ClueID=%s"), *m_ClueID.ToString());
		return false;
	}

	// 조사 명령 인터페이스를 통해 현재 GameMode에 단서 획득을 요청합니다.
	AGameModeBase* GameMode = GetWorld() != nullptr ? GetWorld()->GetAuthGameMode() : nullptr;
	IPFInvestigationCommandReceiver* CommandReceiver = Cast<IPFInvestigationCommandReceiver>(GameMode);
	if (CommandReceiver == nullptr)
	{
		PF_LOG(TEXT("Current GameMode does not provide Investigation commands"));
		return false;
	}

	const EPFClueAcquireResult Result = CommandReceiver->TryAcquireClue(m_ClueID);
	PF_LOG(
		TEXT("Clue interaction completed. ClueID=%s, Result=%s"),
		*m_ClueID.ToString(),
		GetAcquireResultText(Result));

	return Result == EPFClueAcquireResult::Acquired || Result == EPFClueAcquireResult::AlreadyOwned;
}
