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
	 * Investigation 또는 Meeting을 종료하고 PartyFormation으로 전환합니다.
	 * Investigation에서는 Meeting을 생략하고, Meeting에서는 정상 완료 흐름을 사용합니다.
	 */
	bool TryEnterPartyFormation(const FGameplayTag& CaseID);
	bool TryConfirmParty(
		const FGameplayTag& CaseID,
		const FPFPartyData& Party);
private:
	void ResetActiveCase();
	void UnregisterAllMessages();

	void HandleInvestigationReady(FGameplayTag Channel,const FPFInvestigationReadyMessage& Message);
	void HandleInvestigationFinished(FGameplayTag Channel,const FPFInvestigationResult& Message);
	void HandleMeetingCompleted(FGameplayTag Channel, const FPFMeetingResult& Message);
	void HandleBattleReady(FGameplayTag Channel, const FPFBattleReadyMessage& Message);
	void HandleBattleCompleted(FGameplayTag Channel, const FPFBattleResult& Message);
	void TransitionToPartyFormation(const FPFMeetingResult& MeetingResult);

	bool RegisterInvestigationReadyMessage();
	bool RegisterBattleReadyMessage();
private:
	friend struct FPFGameFlowSubsystemAutomationTestAccessor;

	FPFActiveCaseState m_ActiveCaseState;
	
	FGameplayMessageListenerHandle m_InvestigationCompletedMessageHandle;
	FGameplayMessageListenerHandle m_InvestigationSkippedMessageHandle;
	FGameplayMessageListenerHandle m_InvestigationReadyMessageHandle;
	FGameplayMessageListenerHandle m_MeetingCompletedMessageHandle;
	FGameplayMessageListenerHandle m_BattleReadyMessageHandle;
	FGameplayMessageListenerHandle m_BattleCompletedMessageHandle;
};
