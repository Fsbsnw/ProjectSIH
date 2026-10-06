#include "PFDebugFlowModal.h"

#include "Engine/GameInstance.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/000_GameFlow/PFGameFlowSubsystem.h"

bool UPFDebugFlowModal::TrySkipInvestigation()
{
	UGameInstance* GameInstance = GetGameInstance();
	UPFGameFlowSubsystem* GameFlow = IsValid(GameInstance)
		? GameInstance->GetSubsystem<UPFGameFlowSubsystem>()
		: nullptr;

	if (!IsValid(GameFlow))
	{
		PF_LOG(TEXT("GameFlowSubsystem is not available"));
		return false;
	}

	const FPFActiveCaseState& CaseState = GameFlow->GetActiveCaseState();
	if (!CaseState.IsActiveInPhase(EPFCasePhase::Investigation)
		|| !GameFlow->TryEnterPartyFormation(CaseState.m_CaseID))
	{
		PF_LOG(TEXT("Debug investigation skip was rejected"));
		return false;
	}

	DeactivateWidget();
	return true;
}
