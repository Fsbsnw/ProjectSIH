#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Project_SIH/000_Core/001_Contracts/000_Flow/PFGameFlowTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PFGameFlowSubsystem.generated.h"


struct FPFInvestigationReadyMessage;
struct FPFInvestigationResult;
struct FPFMeetingResult;
struct FPFBattleReadyMessage;
struct FPFBattleResult;

UCLASS()
class PROJECT_SIH_API UPFGameFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	const FPFActiveCaseState& GetActiveCaseState() const;

	EPFPhaseStartResult StartCase(
		const FGameplayTag& CaseID);
	bool TryCloseCase(const FGameplayTag& CaseID);

	/**
	 * 현재 Phase의 종료를 요청합니다.
	 * Investigation에서는 Meeting으로, Meeting에서는 PartyFormation으로 진행합니다.
	 */
	bool TryEnterPartyFormation(const FGameplayTag& CaseID);
	bool TryConfirmParty(
		const FGameplayTag& CaseID,
		const FPFPartyData& Party);
private:
	// MVP용 시작 캐릭터 주입.
	// SaveLoad와 캐릭터 소유 흐름이 생기면 이 설정 기반 초기화 경로를 교체한다.
	bool TryInitializeCharacterStates(
		const TArray<FGameplayTag>& InitialCharacterIDs);

	void ResetActiveCase();
	void UnregisterAllMessages();

	void HandleInvestigationReady(FGameplayTag Channel,const FPFInvestigationReadyMessage& Message);
	void HandleInvestigationFinished(FGameplayTag Channel,const FPFInvestigationResult& Message);
	void HandleMeetingCompleted(FGameplayTag Channel, const FPFMeetingResult& Message);
	void HandleBattleReady(FGameplayTag Channel, const FPFBattleReadyMessage& Message);
	void HandleBattleCompleted(FGameplayTag Channel, const FPFBattleResult& Message);
	void TransitionToPartyFormation(const FPFMeetingResult& MeetingResult);
	bool TryShowMeetingDialogueWidget();

	bool RegisterInvestigationReadyMessage();
	bool RegisterBattleReadyMessage();
private:
	friend struct FPFGameFlowSubsystemAutomationTestAccessor;
	friend struct FPFBattleRuntimeGameFlowTestAccessor;

	FPFActiveCaseState m_ActiveCaseState;
	
	FGameplayMessageListenerHandle m_InvestigationCompletedMessageHandle;
	FGameplayMessageListenerHandle m_InvestigationSkippedMessageHandle;
	FGameplayMessageListenerHandle m_InvestigationReadyMessageHandle;
	FGameplayMessageListenerHandle m_MeetingCompletedMessageHandle;
	FGameplayMessageListenerHandle m_BattleReadyMessageHandle;
	FGameplayMessageListenerHandle m_BattleCompletedMessageHandle;
};
