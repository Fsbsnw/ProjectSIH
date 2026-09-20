#include "PFGameFlowSubsystem.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "PFGameFlowSettings.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFInvestigationMessages.h"
#include "Project_SIH/000_Core/001_Contracts/001_Investigation/PFMeetingMessages.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleMessages.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"
#include "Project_SIH/002_Systems/001_Investigation/Interfaces/PFInvestigationCommandReceiver.h"
#include "Project_SIH/002_Systems/001_Investigation/Interfaces/PFInvestigationEntryReceiver.h"
#include "Project_SIH/002_Systems/001_Investigation/Interfaces/PFMeetingCommandReceiver.h"
#include "Project_SIH/002_Systems/001_Investigation/Interfaces/PFMeetingEntryReceiver.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFBattleEntryReceiver.h"
#include "Project_SIH/002_Systems/004_CharacterState/PFCharacterStateSubsystem.h"

void UPFGameFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!Collection.InitializeDependency<
		UPFCharacterStateSubsystem>())
	{
		PF_LOG(TEXT("CharacterStateSubsystem is not valid"));
		return;
	}

	UGameplayMessageSubsystem* MessageSubsystem =
		Collection.InitializeDependency<UGameplayMessageSubsystem>();
	if (!MessageSubsystem)
	{
		PF_LOG(TEXT("GameplayMessageSubsystem is not valid"));
		return;
	}

	m_InvestigationCompletedMessageHandle = MessageSubsystem->RegisterListener<FPFInvestigationResult>(
		SIHGameplayTags::Message_Flow_Investigation_Completed.GetTag(),
		this,
		&UPFGameFlowSubsystem::HandleInvestigationFinished
	);

	m_InvestigationSkippedMessageHandle = MessageSubsystem->RegisterListener<FPFInvestigationResult>(
		SIHGameplayTags::Message_Flow_Investigation_Skipped.GetTag(),
		this,
		&UPFGameFlowSubsystem::HandleInvestigationFinished
	);

	m_MeetingCompletedMessageHandle = MessageSubsystem->RegisterListener<FPFMeetingResult>(
		SIHGameplayTags::Message_Flow_Meeting_Completed.GetTag(),
		this,
		&UPFGameFlowSubsystem::HandleMeetingCompleted
	);

	m_BattleCompletedMessageHandle = MessageSubsystem->RegisterListener<FPFBattleResult>(
		SIHGameplayTags::Message_Flow_Battle_Completed.GetTag(),
		this,
		&UPFGameFlowSubsystem::HandleBattleCompleted
	);

	// PF_LOG(
	// 	TEXT("Initialized Subsystem = %s, GameInstance = %s, InvestigationListener = %s"),
	// 	*GetNameSafe(this),
	// 	*GetNameSafe(GetGameInstance()),
	// 	m_InvestigationCompletedMessageHandle.IsValid()
	// 	? TEXT("valid") : TEXT("invalid"));
}

void UPFGameFlowSubsystem::Deinitialize()
{
	UnregisterAllMessages();
	m_ActiveCaseState = FPFActiveCaseState{};

	Super::Deinitialize();
}

const FPFActiveCaseState& UPFGameFlowSubsystem::GetActiveCaseState() const
{
	return m_ActiveCaseState;
}

bool UPFGameFlowSubsystem::TryCloseCase(
	const FGameplayTag& CaseID)
{
	if (!m_ActiveCaseState.MatchesCaseAndPhase(
		CaseID,
		EPFCasePhase::Result))
	{
		PF_LOG(
			TEXT(
				"Case state does not match. "
				"RequestedCaseID=%s, "
				"ActiveCaseID=%s, ActivePhase=%d"),
			*CaseID.ToString(),
			*m_ActiveCaseState.m_CaseID.ToString(),
			static_cast<uint8>(
				m_ActiveCaseState.m_CasePhase));

		return false;
	}

	ResetActiveCase();
	return true;
}

bool UPFGameFlowSubsystem::TryEnterPartyFormation(const FGameplayTag& CaseID)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	AGameModeBase* CurrentGameMode = World->GetAuthGameMode();
	if (!IsValid(CurrentGameMode))
	{
		return false;
	}

	if (m_ActiveCaseState.MatchesCaseAndPhase(CaseID, EPFCasePhase::Investigation))
	{
		IPFInvestigationCommandReceiver* Receiver =
			Cast<IPFInvestigationCommandReceiver>(CurrentGameMode);
		return Receiver && Receiver->TrySkipInvestigation(CaseID);
	}

	if (m_ActiveCaseState.MatchesCaseAndPhase(CaseID, EPFCasePhase::Meeting))
	{
		IPFMeetingCommandReceiver* Receiver =
			Cast<IPFMeetingCommandReceiver>(CurrentGameMode);
		return Receiver && Receiver->TryCompleteMeeting(CaseID);
	}

	return false;
}

void UPFGameFlowSubsystem::ResetActiveCase()
{
	m_InvestigationReadyMessageHandle.Unregister();
	m_BattleReadyMessageHandle.Unregister();

	m_ActiveCaseState = FPFActiveCaseState{};
}

void UPFGameFlowSubsystem::UnregisterAllMessages()
{
	m_InvestigationReadyMessageHandle.Unregister();
	m_InvestigationCompletedMessageHandle.Unregister();
	m_InvestigationSkippedMessageHandle.Unregister();
	m_MeetingCompletedMessageHandle.Unregister();
	m_BattleReadyMessageHandle.Unregister();
	m_BattleCompletedMessageHandle.Unregister();
}

bool UPFGameFlowSubsystem::TryConfirmParty(
	const FGameplayTag& CaseID,
	const FPFPartyData& Party)
{
	if (!m_ActiveCaseState.MatchesCaseAndPhase(
		CaseID,
		EPFCasePhase::PartyFormation))
	{
		PF_LOG(
			TEXT(
				"Case state does not match. "
				"RequestedCaseID=%s, "
				"ActiveCaseID=%s, ActivePhase=%d"),
			*CaseID.ToString(),
			*m_ActiveCaseState.m_CaseID.ToString(),
			static_cast<uint8>(
				m_ActiveCaseState.m_CasePhase));
		return false;
	}

	if (!Party.IsValid())
	{
		PF_LOG(TEXT("Party is invalid"));
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		PF_LOG(TEXT("GameInstance is not valid"));
		return false;
	}

	const UPFCharacterStateSubsystem* CharacterStateSubsystem =
		GameInstance->GetSubsystem<
			UPFCharacterStateSubsystem>();

	if (!CharacterStateSubsystem)
	{
		PF_LOG(TEXT("CharacterStateSubsystem is not valid"));
		return false;
	}

	UPFAssetManager& AssetManager = UPFAssetManager::Get();
	const FGameplayTag CharacterRoot = SIHGameplayTags::ID_Character.GetTag();
	for (const FGameplayTag& CharacterID : Party.m_CharacterIDs)
	{
		if (!CharacterID.MatchesTag(CharacterRoot) ||
			CharacterID == CharacterRoot)
		{
			PF_LOG(
				TEXT("Invalid party CharacterID : %s"),
				*CharacterID.ToString());
			return false;
		}

		if (!AssetManager.GetCharacterDefinition(CharacterID))
		{
			PF_LOG(
				TEXT(
					"CharacterDefinition is not available. "
					"CharacterID=%s"),
				*CharacterID.ToString());
			return false;
		}

		FPFCharacterStateData CharacterState;
		if (!CharacterStateSubsystem->TryGetCharacterState(
			CharacterID,
			CharacterState))
		{
			PF_LOG(
				TEXT(
					"Character is not available for party. "
					"CharacterID=%s"),
				*CharacterID.ToString());
			return false;
		}
	}

	const UPFGameFlowSettings* Settings =
		GetDefault<UPFGameFlowSettings>();

	const TSoftObjectPtr<UWorld>& BattleMap =
		Settings->GetBattleMap();

	if (BattleMap.IsNull())
	{
		PF_LOG(TEXT("Battle Map is not configured"));
		return false;
	}

	if (!GetWorld())
	{
		PF_LOG(TEXT("World is not valid"));
		return false;
	}

	if (!RegisterBattleReadyMessage())
	{
		PF_LOG(TEXT("Failed to register Battle Ready listener"));
		return false;
	}

	m_ActiveCaseState.m_CurrentParty = Party;

	// UGameInstance* GameInstance = GetGameInstance();

	// PF_LOG(
	// 	TEXT("[BattleTravelSmoke] Requesting travel. "
	// 		 "CaseID=%s, Members=%d, Phase=%d, "
	// 		 "GameInstance=%s[%u], GameFlow=%s[%u], Listener=%s"),
	// 	*m_ActiveCaseState.m_CaseID.ToString(),
	// 	m_ActiveCaseState.m_CurrentParty.m_CharacterIDs.Num(),
	// 	static_cast<uint8>(m_ActiveCaseState.m_CasePhase),
	// 	*GetNameSafe(GameInstance),
	// 	GameInstance->GetUniqueID(),
	// 	*GetNameSafe(this),
	// 	GetUniqueID(),
	// 	m_BattleReadyMessageHandle.IsValid() ? TEXT("valid") : TEXT("invalid"));

	UGameplayStatics::OpenLevelBySoftObjectPtr(
		this,
		BattleMap);

	return true;
}

EPFPhaseStartResult UPFGameFlowSubsystem::StartCase(
	const FGameplayTag& CaseID)
{
	if (!CaseID.IsValid())
	{
		PF_LOG(TEXT("CaseID is not valid"));
		return EPFPhaseStartResult::InvalidContext;
	}

	if (m_ActiveCaseState.m_CaseID.IsValid())
	{
		PF_LOG(TEXT("A case is already active"));
		return EPFPhaseStartResult::AlreadyActive;
	}

	if (!UPFAssetManager::Get()
		.GetCaseDefinition(CaseID))
	{
		PF_LOG(TEXT("CaseDefinition is not valid"));
		return EPFPhaseStartResult::InvalidContext;
	}

	if (!GetWorld())
	{
		PF_LOG(TEXT("World is not valid"));
		return EPFPhaseStartResult::NotReady;
	}

	const UPFGameFlowSettings* Settings =
		GetDefault<UPFGameFlowSettings>();

	const TSoftObjectPtr<UWorld>& InvestigationMap =
		Settings->GetInvestigationMap();

	if (InvestigationMap.IsNull())
	{
		PF_LOG(
			TEXT("Investigation Map is not configured"));
		return EPFPhaseStartResult::NotReady;
	}

	if (!RegisterInvestigationReadyMessage())
	{
		PF_LOG(
			TEXT(
				"Failed to register "
				"Investigation Ready listener"));
		return EPFPhaseStartResult::NotReady;
	}

	m_ActiveCaseState = FPFActiveCaseState{};
	m_ActiveCaseState.m_CaseID = CaseID;
	m_ActiveCaseState.m_CasePhase =
		EPFCasePhase::Investigation;

	UGameplayStatics::OpenLevelBySoftObjectPtr(
		this,
		InvestigationMap);

	return EPFPhaseStartResult::Started;
}

void UPFGameFlowSubsystem::HandleInvestigationReady(
	FGameplayTag Channel,
	const FPFInvestigationReadyMessage& Message)
{
	m_InvestigationReadyMessageHandle.Unregister();

	if (!m_ActiveCaseState.IsActiveInPhase(
		EPFCasePhase::Investigation))
	{
		PF_LOG(
			TEXT(
				"There is no pending "
				"Investigation start"));

		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		PF_LOG(TEXT("World is not valid"));
		ResetActiveCase();
		return;
	}

	AGameModeBase* CurrentGameMode =
		World->GetAuthGameMode();

	IPFInvestigationEntryReceiver* Receiver =
		Cast<IPFInvestigationEntryReceiver>(
			CurrentGameMode);

	if (!Receiver)
	{
		PF_LOG(
			TEXT(
				"Investigation Entry Receiver "
				"is not available"));
		ResetActiveCase();
		return;
	}

	FPFInvestigationEntryContext Context{};
	Context.m_CaseID =
		m_ActiveCaseState.m_CaseID;

	const EPFPhaseStartResult StartResult =
		Receiver->StartInvestigation(Context);

	if (StartResult
		!= EPFPhaseStartResult::Started)
	{
		PF_LOG(
			TEXT(
				"Investigation start rejected. "
				"Result=%d"),
			static_cast<uint8>(StartResult));

		ResetActiveCase();
		return;
	}

}

void UPFGameFlowSubsystem::HandleInvestigationFinished(FGameplayTag Channel,
                                                        const FPFInvestigationResult& Message)
{
	if (Channel != SIHGameplayTags::Message_Flow_Investigation_Completed.GetTag()
		&& Channel != SIHGameplayTags::Message_Flow_Investigation_Skipped.GetTag())
	{
		PF_LOG(
			TEXT("Unexpected Investigation result channel. Channel=%s"),
			*Channel.ToString());
		return;
	}

	if (!Message.IsValid())
	{
		PF_LOG(TEXT("Investigation result is invalid"));
		return;
	}

	if (!m_ActiveCaseState.m_CaseID.IsValid())
	{
		PF_LOG(TEXT("Their is no active case"));
		return;
	}

	if (m_ActiveCaseState.m_CaseID != Message.m_CaseID)
	{
		PF_LOG(TEXT("Message Case ID is not matching with current casestate"));
		return;
	}

	if (m_ActiveCaseState.m_CasePhase != EPFCasePhase::Investigation)
	{
		PF_LOG(TEXT("CasePhase is not matching with current casestate"));
		return;
	}

	if (Channel == SIHGameplayTags::Message_Flow_Investigation_Skipped.GetTag())
	{
		m_ActiveCaseState.m_InvestigationResult = Message;

		FPFMeetingResult EmptyMeetingResult{};
		EmptyMeetingResult.m_CaseID = Message.m_CaseID;
		TransitionToPartyFormation(EmptyMeetingResult);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		PF_LOG(TEXT("World is not valid"));
		return;
	}

	AGameModeBase* CurrentGameMode = World->GetAuthGameMode();
	if (!CurrentGameMode)
	{
		PF_LOG(TEXT("CurrentGameMode is not valid"));
		return;
	}

	IPFMeetingEntryReceiver* MeetingEntryReceiver = Cast<IPFMeetingEntryReceiver>(CurrentGameMode);
	if (!MeetingEntryReceiver)
	{
		PF_LOG(TEXT("Current GameMode does not implement MeetingEntryReceiver"));
		return;
	}

	FPFMeetingEntryContext MeetingEntryContext{};
	m_ActiveCaseState.m_InvestigationResult = Message;

	MeetingEntryContext.m_CaseID =
		m_ActiveCaseState.m_InvestigationResult.m_CaseID;
	MeetingEntryContext.m_AcquiredClueIDs =
		m_ActiveCaseState.m_InvestigationResult.m_AcquiredClueIDs;

	m_ActiveCaseState.m_CasePhase = EPFCasePhase::Meeting;

	const EPFPhaseStartResult StartResult = MeetingEntryReceiver->StartMeeting(MeetingEntryContext);
	if (StartResult != EPFPhaseStartResult::Started)
	{
		PF_LOG(
			TEXT("Meeting start rejected. Result=%d"),
			static_cast<uint8>(StartResult));
	}
}

void UPFGameFlowSubsystem::HandleMeetingCompleted(FGameplayTag Channel,
                                                  const FPFMeetingResult& Message)
{
	if (!Message.IsValid())
	{
		PF_LOG(TEXT("Meeting result is invalid"));
		return;
	}

	if (!m_ActiveCaseState.m_CaseID.IsValid())
	{
		PF_LOG(TEXT("There is no active case"));
		return;
	}

	if (m_ActiveCaseState.m_CaseID != Message.m_CaseID)
	{
		PF_LOG(TEXT("Message Case ID does not match the active case"));
		return;
	}

	if (m_ActiveCaseState.m_CasePhase != EPFCasePhase::Meeting)
	{
		PF_LOG(TEXT("Current phase is not Meeting"));
		return;
	}

	// PF_LOG(
	// 	TEXT("CurrentPhase : %d"),
	// 	static_cast<uint8>(m_ActiveCaseState.m_CasePhase));

	TransitionToPartyFormation(Message);

	// PF_LOG(
	// 	TEXT("ChangedPhase : %d"),
	// 	static_cast<uint8>(m_ActiveCaseState.m_CasePhase));
}

void UPFGameFlowSubsystem::TransitionToPartyFormation(
	const FPFMeetingResult& MeetingResult)
{
	m_ActiveCaseState.m_MeetingResult = MeetingResult;
	m_ActiveCaseState.m_CasePhase = EPFCasePhase::PartyFormation;
}

void UPFGameFlowSubsystem::HandleBattleReady(
	FGameplayTag Channel,
	const FPFBattleReadyMessage& Message)
{
	m_BattleReadyMessageHandle.Unregister();

	if (!m_ActiveCaseState.m_CaseID.IsValid())
	{
		PF_LOG(TEXT("There is no active case"));
		return;
	}

	if (m_ActiveCaseState.m_CasePhase != EPFCasePhase::PartyFormation)
	{
		PF_LOG(TEXT("Current phase is not PartyFormation"));
		return;
	}

	if (!m_ActiveCaseState.m_CurrentParty.IsValid())
	{
		PF_LOG(TEXT("Current party is invalid"));
		return;
	}

	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	if (!World || !GameInstance)
	{
		PF_LOG(TEXT("World or GameInstance is not valid"));
		return;
	}

	AGameModeBase* CurrentGameMode = World->GetAuthGameMode();
	if (!CurrentGameMode)
	{
		PF_LOG(TEXT("CurrentGameMode is not valid"));
		return;
	}

	IPFBattleEntryReceiver* BattleEntryReceiver =
		Cast<IPFBattleEntryReceiver>(CurrentGameMode);
	if (!BattleEntryReceiver)
	{
		PF_LOG(TEXT("Current GameMode does not implement BattleEntryReceiver"));
		return;
	}

	FPFBattleEntryContext Context;
	Context.m_CaseID = m_ActiveCaseState.m_CaseID;
	Context.m_Party = m_ActiveCaseState.m_CurrentParty;
	Context.m_RevealedWeaknessIDs =
		m_ActiveCaseState.m_MeetingResult.m_RevealedWeaknessIDs;

	const EPFPhaseStartResult StartResult =
		BattleEntryReceiver->StartBattle(Context);

	// PF_LOG(
	// 	TEXT("Battle start result : %d"),
	// 	static_cast<uint8>(StartResult));

	if (StartResult != EPFPhaseStartResult::Started)
	{
		PF_LOG(
			TEXT("Battle start rejected. Result=%d"),
			static_cast<uint8>(StartResult));
		return;
	}

	m_ActiveCaseState.m_CasePhase = EPFCasePhase::Battle;

	// PF_LOG(
	// 	TEXT("[BattleTravelSmoke] Ready received. "
	// 		 "World=%s, CaseID=%s, Members=%d, Phase=%d, "
	// 		 "GameInstance=%s[%u], GameFlow=%s[%u], Listener=%s"),
	// 	*World->GetMapName(),
	// 	*m_ActiveCaseState.m_CaseID.ToString(),
	// 	m_ActiveCaseState.m_CurrentParty.m_CharacterIDs.Num(),
	// 	static_cast<uint8>(m_ActiveCaseState.m_CasePhase),
	// 	*GetNameSafe(GameInstance),
	// 	GameInstance->GetUniqueID(),
	// 	*GetNameSafe(this),
	// 	GetUniqueID(),
	// 	m_BattleReadyMessageHandle.IsValid() ? TEXT("valid") : TEXT("invalid"));
	//
	// for (const FGameplayTag& CharacterID :
	// 	m_ActiveCaseState.m_CurrentParty.m_CharacterIDs)
	// {
	// 	PF_LOG(
	// 		TEXT("[BattleTravelSmoke] Member=%s"),
	// 		*CharacterID.ToString());
	// }
}

void UPFGameFlowSubsystem::HandleBattleCompleted(
	FGameplayTag Channel,
	const FPFBattleResult& Message)
{
	if (!Message.IsValid())
	{
		PF_LOG(TEXT("Battle result is invalid"));
		return;
	}

	if (!m_ActiveCaseState.m_CaseID.IsValid())
	{
		PF_LOG(TEXT("There is no active case"));
		return;
	}

	if (m_ActiveCaseState.m_CaseID != Message.m_CaseID)
	{
		PF_LOG(TEXT("Message CaseID does not match the active case"));
		return;
	}

	if (m_ActiveCaseState.m_CasePhase != EPFCasePhase::Battle)
	{
		PF_LOG(TEXT("Current phase is not Battle"));
		return;
	}

	m_ActiveCaseState.m_BattleResult = Message;
	m_ActiveCaseState.m_CasePhase = EPFCasePhase::Result;

	// PF_LOG(
	// 	TEXT("[BattleResultSmoke] CaseID=%s, ResultType=%d, Phase=%d"),
	// 	*m_ActiveCaseState.m_BattleResult.m_CaseID.ToString(),
	// 	static_cast<uint8>(m_ActiveCaseState.m_BattleResult.m_ResultType),
	// 	static_cast<uint8>(m_ActiveCaseState.m_CasePhase));
}

bool UPFGameFlowSubsystem::
	RegisterInvestigationReadyMessage()
{
	if (m_InvestigationReadyMessageHandle.IsValid())
	{
		PF_LOG(TEXT("Investigation travel is already pending"));
		return false;
	}

	UGameInstance* GameInstance =
		GetGameInstance();

	if (!GameInstance)
	{
		PF_LOG(TEXT("GameInstance is not valid"));
		return false;
	}

	UGameplayMessageSubsystem& MessageSubsystem =
		UGameplayMessageSubsystem::Get(GameInstance);

	m_InvestigationReadyMessageHandle =
		MessageSubsystem.RegisterListener<
			FPFInvestigationReadyMessage>(
			SIHGameplayTags::
				Message_Flow_Investigation_Ready
					.GetTag(),
			this,
			&UPFGameFlowSubsystem::
				HandleInvestigationReady);

	return m_InvestigationReadyMessageHandle.IsValid();
}

bool UPFGameFlowSubsystem::
	RegisterBattleReadyMessage()
{
	if (m_BattleReadyMessageHandle.IsValid())
	{
		PF_LOG(TEXT("Battle travel is already pending"));
		return false;
	}

	UGameInstance* GameInstance =
		GetGameInstance();

	if (!GameInstance)
	{
		PF_LOG(TEXT("GameInstance is not valid"));
		return false;
	}

	UGameplayMessageSubsystem& MessageSubsystem =
		UGameplayMessageSubsystem::Get(GameInstance);

	m_BattleReadyMessageHandle =
		MessageSubsystem.RegisterListener<
			FPFBattleReadyMessage>(
			SIHGameplayTags::
				Message_Flow_Battle_Ready
					.GetTag(),
			this,
			&UPFGameFlowSubsystem::
				HandleBattleReady);

	return m_BattleReadyMessageHandle.IsValid();
}
