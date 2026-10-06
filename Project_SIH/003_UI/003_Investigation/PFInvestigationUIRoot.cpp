#include "PFInvestigationUIRoot.h"

#include "Engine/GameInstance.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/000_GameFlow/PFGameFlowSubsystem.h"
#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/SIHGameplayTags.h"

bool UPFInvestigationUIRoot::TryStartCase(FGameplayTag CaseID)
{
	if (!PFGameplayTagUtilities::IsValidChildTag(CaseID, SIHGameplayTags::ID_Case.GetTag()))
	{
		PF_LOG(TEXT("CaseID is outside the Case ID domain. CaseID=%s"), *CaseID.ToString());
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UPFGameFlowSubsystem* GameFlow = IsValid(GameInstance)
		? GameInstance->GetSubsystem<UPFGameFlowSubsystem>()
		: nullptr;
	if (!IsValid(GameFlow))
	{
		PF_LOG(TEXT("GameFlowSubsystem is not available."));
		return false;
	}

	const EPFPhaseStartResult StartResult = GameFlow->StartCase(CaseID);
	if (StartResult != EPFPhaseStartResult::Started)
	{
		PF_LOG(
			TEXT("Case start request was rejected. CaseID=%s, Result=%d"),
			*CaseID.ToString(),
			static_cast<uint8>(StartResult));
		return false;
	}

	return true;
}
