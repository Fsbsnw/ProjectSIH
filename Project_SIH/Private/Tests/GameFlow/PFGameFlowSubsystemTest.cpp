#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "Project_SIH/000_Core/001_Contracts/000_Flow/PFGameFlowTypes.h"
#include "Project_SIH/002_Systems/000_GameFlow/PFGameFlowSubsystem.h"
#include "Project_SIH/SIHGameplayTags.h"

struct FPFGameFlowSubsystemAutomationTestAccessor
{
	static void SetActiveCaseState(
		UPFGameFlowSubsystem& GameFlowSubsystem,
		const FPFActiveCaseState& ActiveCaseState)
	{
		GameFlowSubsystem.m_ActiveCaseState =
			ActiveCaseState;
	}

	static void DispatchMeetingCompleted(
		UPFGameFlowSubsystem& GameFlowSubsystem,
		const FGameplayTag& Channel,
		const FPFMeetingResult& Message)
	{
		GameFlowSubsystem.HandleMeetingCompleted(
			Channel,
			Message);
	}

	static void DispatchInvestigationFinished(
		UPFGameFlowSubsystem& GameFlowSubsystem,
		const FGameplayTag& Channel,
		const FPFInvestigationResult& Message)
	{
		GameFlowSubsystem.HandleInvestigationFinished(
			Channel,
			Message);
	}

	static void DispatchBattleCompleted(
		UPFGameFlowSubsystem& GameFlowSubsystem,
		const FGameplayTag& Channel,
		const FPFBattleResult& Message)
	{
		GameFlowSubsystem.HandleBattleCompleted(
			Channel,
			Message);
	}
};

namespace
{
	FPFActiveCaseState CreateActiveCaseState(
		const EPFCasePhase CasePhase)
	{
		FPFActiveCaseState ActiveCaseState;
		ActiveCaseState.m_CaseID =
			SIHGameplayTags::ID_Case_FlowTest.GetTag();
		ActiveCaseState.m_CasePhase = CasePhase;
		ActiveCaseState.m_CurrentParty.m_CharacterIDs.AddTag(
			SIHGameplayTags::ID_Character_TestCharacter.GetTag());

		if (CasePhase == EPFCasePhase::Result)
		{
			ActiveCaseState.m_InvestigationResult.m_CaseID =
				ActiveCaseState.m_CaseID;
			ActiveCaseState.m_InvestigationResult
				.m_AcquiredClueIDs.AddTag(
					SIHGameplayTags::ID_Clue_FlowTest.GetTag());

			ActiveCaseState.m_MeetingResult.m_CaseID =
				ActiveCaseState.m_CaseID;
			ActiveCaseState.m_MeetingResult
				.m_RevealedWeaknessIDs.Add(
					SIHGameplayTags::ID_Element_FlowTest.GetTag());

			ActiveCaseState.m_BattleResult.m_CaseID =
				ActiveCaseState.m_CaseID;
			ActiveCaseState.m_BattleResult.m_ResultType =
				EPFBattleResult::NormalVictory;
		}

		return ActiveCaseState;
	}

	bool AreActiveCaseStatesEqual(
		const FPFActiveCaseState& Left,
		const FPFActiveCaseState& Right)
	{
		return Left.m_CaseID == Right.m_CaseID
			&& Left.m_CasePhase == Right.m_CasePhase
			&& Left.m_InvestigationResult.m_CaseID
				== Right.m_InvestigationResult.m_CaseID
			&& Left.m_InvestigationResult.m_AcquiredClueIDs
				== Right.m_InvestigationResult.m_AcquiredClueIDs
			&& Left.m_MeetingResult.m_CaseID
				== Right.m_MeetingResult.m_CaseID
			&& Left.m_MeetingResult.m_RevealedWeaknessIDs
				== Right.m_MeetingResult.m_RevealedWeaknessIDs
			&& Left.m_CurrentParty.m_CharacterIDs
				== Right.m_CurrentParty.m_CharacterIDs
			&& Left.m_BattleResult.m_CaseID
				== Right.m_BattleResult.m_CaseID
			&& Left.m_BattleResult.m_ResultType
				== Right.m_BattleResult.m_ResultType;
	}

	bool IsActiveCaseStateEmpty(
		const FPFActiveCaseState& ActiveCaseState)
	{
		return !ActiveCaseState.m_CaseID.IsValid()
			&& ActiveCaseState.m_CasePhase
				== EPFCasePhase::None
			&& !ActiveCaseState.m_InvestigationResult.IsValid()
			&& ActiveCaseState.m_InvestigationResult
				.m_AcquiredClueIDs.IsEmpty()
			&& !ActiveCaseState.m_MeetingResult.IsValid()
			&& ActiveCaseState.m_MeetingResult
				.m_RevealedWeaknessIDs.IsEmpty()
			&& ActiveCaseState.m_CurrentParty
				.m_CharacterIDs.IsEmpty()
			&& !ActiveCaseState.m_BattleResult.IsValid();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPFGameFlowActiveCaseLifecycleTest,
	"Project_SIH.GameFlow.ActiveCaseLifecycle",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::ProductFilter)

bool FPFGameFlowActiveCaseLifecycleTest::RunTest(
	const FString& Parameters)
{
	const FGameplayTag CaseID =
		SIHGameplayTags::ID_Case_FlowTest.GetTag();
	const FGameplayTag OtherCaseID =
		SIHGameplayTags::ID_Case.GetTag();

	const FPFActiveCaseState InvestigationState =
		CreateActiveCaseState(
			EPFCasePhase::Investigation);

	TestTrue(
		TEXT("같은 Case와 Phase는 일치해야 합니다."),
		InvestigationState.MatchesCaseAndPhase(
			CaseID,
			EPFCasePhase::Investigation));

	TestFalse(
		TEXT("유효하지 않은 CaseID는 일치하지 않아야 합니다."),
		InvestigationState.MatchesCaseAndPhase(
			FGameplayTag(),
			EPFCasePhase::Investigation));

	TestFalse(
		TEXT("다른 CaseID는 일치하지 않아야 합니다."),
		InvestigationState.MatchesCaseAndPhase(
			OtherCaseID,
			EPFCasePhase::Investigation));

	TestFalse(
		TEXT("다른 Phase는 일치하지 않아야 합니다."),
		InvestigationState.MatchesCaseAndPhase(
			CaseID,
			EPFCasePhase::Result));

	UGameInstance* GameInstance =
		NewObject<UGameInstance>();
	UPFGameFlowSubsystem* GameFlowSubsystem =
		NewObject<UPFGameFlowSubsystem>(GameInstance);

	if (!TestNotNull(
		TEXT("GameFlowSubsystem 테스트 객체가 생성되어야 합니다."),
		GameFlowSubsystem))
	{
		return false;
	}

	TestFalse(
		TEXT("활성 사건이 없으면 사건 종료에 실패해야 합니다."),
		GameFlowSubsystem->TryCloseCase(CaseID));

	FPFGameFlowSubsystemAutomationTestAccessor::
		SetActiveCaseState(
			*GameFlowSubsystem,
			InvestigationState);

	TestFalse(
		TEXT("Result 이전에는 사건 종료에 실패해야 합니다."),
		GameFlowSubsystem->TryCloseCase(CaseID));

	TestTrue(
		TEXT("종료 실패 시 기존 ActiveCase를 유지해야 합니다."),
		AreActiveCaseStatesEqual(
			GameFlowSubsystem->GetActiveCaseState(),
			InvestigationState));

	const FPFActiveCaseState ResultState =
		CreateActiveCaseState(
			EPFCasePhase::Result);

	FPFGameFlowSubsystemAutomationTestAccessor::
		SetActiveCaseState(
			*GameFlowSubsystem,
			ResultState);

	TestFalse(
		TEXT("유효하지 않은 CaseID로 사건을 종료할 수 없어야 합니다."),
		GameFlowSubsystem->TryCloseCase(
			FGameplayTag()));

	TestTrue(
		TEXT("유효하지 않은 종료 요청은 ActiveCase를 변경하지 않아야 합니다."),
		AreActiveCaseStatesEqual(
			GameFlowSubsystem->GetActiveCaseState(),
			ResultState));

	TestFalse(
		TEXT("다른 CaseID로 사건을 종료할 수 없어야 합니다."),
		GameFlowSubsystem->TryCloseCase(
			OtherCaseID));

	TestTrue(
		TEXT("다른 Case의 종료 요청은 ActiveCase를 변경하지 않아야 합니다."),
		AreActiveCaseStatesEqual(
			GameFlowSubsystem->GetActiveCaseState(),
			ResultState));

	TestTrue(
		TEXT("Result 상태의 현재 사건은 정상 종료되어야 합니다."),
		GameFlowSubsystem->TryCloseCase(CaseID));

	TestTrue(
		TEXT("사건 종료 후 ActiveCase, Party, Result가 초기화되어야 합니다."),
		IsActiveCaseStateEmpty(
			GameFlowSubsystem->GetActiveCaseState()));

	TestFalse(
		TEXT("이미 종료한 사건을 다시 종료할 수 없어야 합니다."),
		GameFlowSubsystem->TryCloseCase(CaseID));

	FPFGameFlowSubsystemAutomationTestAccessor::
		SetActiveCaseState(
			*GameFlowSubsystem,
			ResultState);

	GameFlowSubsystem->Deinitialize();

	TestTrue(
		TEXT("Subsystem 종료 시 ActiveCase가 초기화되어야 합니다."),
		IsActiveCaseStateEmpty(
			GameFlowSubsystem->GetActiveCaseState()));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPFGameFlowPhaseResultTest,
	"Project_SIH.GameFlow.PhaseResult",
	EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::ProductFilter)

bool FPFGameFlowPhaseResultTest::RunTest(
	const FString& Parameters)
{
	const FGameplayTag CaseID =
		SIHGameplayTags::ID_Case_FlowTest.GetTag();
	const FGameplayTag OtherCaseID =
		SIHGameplayTags::ID_Case.GetTag();

	UGameInstance* GameInstance =
		NewObject<UGameInstance>();
	UPFGameFlowSubsystem* GameFlowSubsystem =
		NewObject<UPFGameFlowSubsystem>(GameInstance);

	if (!TestNotNull(
		TEXT("GameFlowSubsystem 테스트 객체가 생성되어야 합니다."),
		GameFlowSubsystem))
	{
		return false;
	}

	const FPFActiveCaseState InvestigationState =
		CreateActiveCaseState(EPFCasePhase::Investigation);

	FPFGameFlowSubsystemAutomationTestAccessor::
		SetActiveCaseState(
			*GameFlowSubsystem,
			InvestigationState);

	FPFInvestigationResult InvestigationResult;
	InvestigationResult.m_CaseID = CaseID;
	InvestigationResult.m_AcquiredClueIDs.AddTag(
		SIHGameplayTags::ID_Clue_FlowTest.GetTag());

	FPFGameFlowSubsystemAutomationTestAccessor::
		DispatchInvestigationFinished(
			*GameFlowSubsystem,
			SIHGameplayTags::Message.GetTag(),
			InvestigationResult);

	TestTrue(
		TEXT("알 수 없는 Investigation 결과 채널은 상태를 변경하지 않아야 합니다."),
		AreActiveCaseStatesEqual(
			GameFlowSubsystem->GetActiveCaseState(),
			InvestigationState));

	FPFInvestigationResult OtherCaseInvestigationResult =
		InvestigationResult;
	OtherCaseInvestigationResult.m_CaseID = OtherCaseID;

	FPFGameFlowSubsystemAutomationTestAccessor::
		DispatchInvestigationFinished(
			*GameFlowSubsystem,
			SIHGameplayTags::Message_Flow_Investigation_Skipped.GetTag(),
			OtherCaseInvestigationResult);

	TestTrue(
		TEXT("다른 Case의 Investigation Skip 결과는 상태를 변경하지 않아야 합니다."),
		AreActiveCaseStatesEqual(
			GameFlowSubsystem->GetActiveCaseState(),
			InvestigationState));

	FPFGameFlowSubsystemAutomationTestAccessor::
		DispatchInvestigationFinished(
			*GameFlowSubsystem,
			SIHGameplayTags::Message_Flow_Investigation_Skipped.GetTag(),
			InvestigationResult);

	const FPFActiveCaseState& InvestigationSkippedState =
		GameFlowSubsystem->GetActiveCaseState();

	TestTrue(
		TEXT("Investigation Skip 결과는 PartyFormation으로 전환해야 합니다."),
		InvestigationSkippedState.MatchesCaseAndPhase(
			CaseID,
			EPFCasePhase::PartyFormation));
	TestTrue(
		TEXT("Investigation Skip 결과는 획득한 단서를 저장해야 합니다."),
		InvestigationSkippedState.m_InvestigationResult
			.m_AcquiredClueIDs.HasTagExact(
				SIHGameplayTags::ID_Clue_FlowTest.GetTag()));
	TestEqual(
		TEXT("Investigation Skip의 빈 Meeting 결과도 현재 CaseID를 유지해야 합니다."),
		InvestigationSkippedState.m_MeetingResult.m_CaseID,
		CaseID);
	TestTrue(
		TEXT("Investigation Skip의 Meeting 간파 결과는 비어 있어야 합니다."),
		InvestigationSkippedState.m_MeetingResult
			.m_RevealedWeaknessIDs.IsEmpty());

	const FPFActiveCaseState StoredInvestigationSkippedState =
		InvestigationSkippedState;

	FPFGameFlowSubsystemAutomationTestAccessor::
		DispatchInvestigationFinished(
			*GameFlowSubsystem,
			SIHGameplayTags::Message_Flow_Investigation_Skipped.GetTag(),
			InvestigationResult);

	TestTrue(
		TEXT("PartyFormation 이후 늦은 Investigation Skip 결과는 상태를 변경하지 않아야 합니다."),
		AreActiveCaseStatesEqual(
			GameFlowSubsystem->GetActiveCaseState(),
			StoredInvestigationSkippedState));

	FPFMeetingResult MeetingResult;
	MeetingResult.m_CaseID = CaseID;
	MeetingResult.m_RevealedWeaknessIDs.Add(
		SIHGameplayTags::ID_Element_FlowTest.GetTag());

	const FPFActiveCaseState MeetingState =
		CreateActiveCaseState(EPFCasePhase::Meeting);

	FPFGameFlowSubsystemAutomationTestAccessor::
		SetActiveCaseState(
			*GameFlowSubsystem,
			MeetingState);

	FPFMeetingResult OtherCaseMeetingResult = MeetingResult;
	OtherCaseMeetingResult.m_CaseID = OtherCaseID;

	FPFGameFlowSubsystemAutomationTestAccessor::
		DispatchMeetingCompleted(
			*GameFlowSubsystem,
			SIHGameplayTags::Message_Flow_Meeting_Completed.GetTag(),
			OtherCaseMeetingResult);

	TestTrue(
		TEXT("다른 Case의 Meeting 결과는 상태를 변경하지 않아야 합니다."),
		AreActiveCaseStatesEqual(
			GameFlowSubsystem->GetActiveCaseState(),
			MeetingState));

	FPFGameFlowSubsystemAutomationTestAccessor::
		DispatchMeetingCompleted(
			*GameFlowSubsystem,
			SIHGameplayTags::Message_Flow_Meeting_Completed.GetTag(),
			MeetingResult);

	const FPFActiveCaseState& PartyFormationState =
		GameFlowSubsystem->GetActiveCaseState();

	TestTrue(
		TEXT("유효한 Meeting 결과는 PartyFormation으로 전환해야 합니다."),
		PartyFormationState.m_CasePhase
			== EPFCasePhase::PartyFormation);
	TestTrue(
		TEXT("Meeting 간파 결과를 ActiveCase에 저장해야 합니다."),
		PartyFormationState.m_MeetingResult.m_RevealedWeaknessIDs
			.Contains(
				SIHGameplayTags::ID_Element_FlowTest.GetTag()));

	const FPFActiveCaseState StoredMeetingState =
		PartyFormationState;

	FPFGameFlowSubsystemAutomationTestAccessor::
		DispatchMeetingCompleted(
			*GameFlowSubsystem,
			SIHGameplayTags::Message_Flow_Meeting_Completed.GetTag(),
			MeetingResult);

	TestTrue(
		TEXT("중복 Meeting 결과는 저장 상태를 덮어쓰지 않아야 합니다."),
		AreActiveCaseStatesEqual(
			GameFlowSubsystem->GetActiveCaseState(),
			StoredMeetingState));

	const FPFActiveCaseState BattleState =
		CreateActiveCaseState(EPFCasePhase::Battle);

	FPFGameFlowSubsystemAutomationTestAccessor::
		SetActiveCaseState(
			*GameFlowSubsystem,
			BattleState);

	FPFBattleResult InvalidBattleResult;
	InvalidBattleResult.m_CaseID = CaseID;

	FPFGameFlowSubsystemAutomationTestAccessor::
		DispatchBattleCompleted(
			*GameFlowSubsystem,
			SIHGameplayTags::Message_Flow_Battle_Completed.GetTag(),
			InvalidBattleResult);

	TestTrue(
		TEXT("None Battle 결과는 상태를 변경하지 않아야 합니다."),
		AreActiveCaseStatesEqual(
			GameFlowSubsystem->GetActiveCaseState(),
			BattleState));

	FPFBattleResult BattleResult;
	BattleResult.m_CaseID = CaseID;
	BattleResult.m_ResultType =
		EPFBattleResult::NormalVictory;

	FPFBattleResult OtherCaseBattleResult = BattleResult;
	OtherCaseBattleResult.m_CaseID = OtherCaseID;

	FPFGameFlowSubsystemAutomationTestAccessor::
		DispatchBattleCompleted(
			*GameFlowSubsystem,
			SIHGameplayTags::Message_Flow_Battle_Completed.GetTag(),
			OtherCaseBattleResult);

	TestTrue(
		TEXT("다른 Case의 Battle 결과는 상태를 변경하지 않아야 합니다."),
		AreActiveCaseStatesEqual(
			GameFlowSubsystem->GetActiveCaseState(),
			BattleState));

	FPFGameFlowSubsystemAutomationTestAccessor::
		DispatchBattleCompleted(
			*GameFlowSubsystem,
			SIHGameplayTags::Message_Flow_Battle_Completed.GetTag(),
			BattleResult);

	const FPFActiveCaseState& ResultState =
		GameFlowSubsystem->GetActiveCaseState();

	TestTrue(
		TEXT("유효한 Battle 결과는 Result로 전환해야 합니다."),
		ResultState.m_CasePhase
			== EPFCasePhase::Result);
	TestTrue(
		TEXT("Battle 결과 종류를 ActiveCase에 저장해야 합니다."),
		ResultState.m_BattleResult.m_ResultType
			== EPFBattleResult::NormalVictory);

	const FPFActiveCaseState StoredResultState =
		ResultState;
	FPFBattleResult LateBattleResult = BattleResult;
	LateBattleResult.m_ResultType =
		EPFBattleResult::ForcedEviction;

	FPFGameFlowSubsystemAutomationTestAccessor::
		DispatchBattleCompleted(
			*GameFlowSubsystem,
			SIHGameplayTags::Message_Flow_Battle_Completed.GetTag(),
			LateBattleResult);

	TestTrue(
		TEXT("Result 이후 늦은 Battle 결과는 저장 상태를 덮어쓰지 않아야 합니다."),
		AreActiveCaseStatesEqual(
			GameFlowSubsystem->GetActiveCaseState(),
			StoredResultState));

	return true;
}

#endif
