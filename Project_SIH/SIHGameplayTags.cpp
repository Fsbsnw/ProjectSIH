#include "SIHGameplayTags.h"

namespace SIHGameplayTags
{
	#pragma region Content ID Tags

	UE_DEFINE_GAMEPLAY_TAG(ID, "ID");
	UE_DEFINE_GAMEPLAY_TAG(ID_Character, "ID.Character");
	UE_DEFINE_GAMEPLAY_TAG(ID_Character_TestCharacter, "ID.Character.TestCharacter");
	UE_DEFINE_GAMEPLAY_TAG(ID_Character_EditorImportTest, "ID.Character.EditorImportTest");
	UE_DEFINE_GAMEPLAY_TAG(ID_Character_PartyTestA, "ID.Character.PartyTestA");
	UE_DEFINE_GAMEPLAY_TAG(ID_Character_PartyTestB, "ID.Character.PartyTestB");
	UE_DEFINE_GAMEPLAY_TAG(
		ID_Character_PartyMissingDefinition,
		"ID.Character.PartyMissingDefinition");
	UE_DEFINE_GAMEPLAY_TAG(ID_Item, "ID.Item");
	UE_DEFINE_GAMEPLAY_TAG(ID_Item_TestEquipment, "ID.Item.TestEquipment");
	UE_DEFINE_GAMEPLAY_TAG(ID_Skill, "ID.Skill");
	UE_DEFINE_GAMEPLAY_TAG(ID_Case, "ID.Case");
	UE_DEFINE_GAMEPLAY_TAG(ID_Case_FlowTest, "ID.Case.FlowTest");
	UE_DEFINE_GAMEPLAY_TAG(ID_Clue, "ID.Clue");
	UE_DEFINE_GAMEPLAY_TAG(ID_Clue_FlowTest, "ID.Clue.FlowTest");
#if WITH_DEV_AUTOMATION_TESTS
	UE_DEFINE_GAMEPLAY_TAG(
		ID_Clue_CombinationTest_SourceA,
		"ID.Clue.CombinationTest.SourceA");
	UE_DEFINE_GAMEPLAY_TAG(
		ID_Clue_CombinationTest_SourceB,
		"ID.Clue.CombinationTest.SourceB");
	UE_DEFINE_GAMEPLAY_TAG(
		ID_Clue_CombinationTest_SourceC,
		"ID.Clue.CombinationTest.SourceC");
	UE_DEFINE_GAMEPLAY_TAG(
		ID_Clue_CombinationTest_ResultAB,
		"ID.Clue.CombinationTest.ResultAB");
	UE_DEFINE_GAMEPLAY_TAG(
		ID_Clue_CombinationTest_ResultABC,
		"ID.Clue.CombinationTest.ResultABC");
	UE_DEFINE_GAMEPLAY_TAG(
		ID_Clue_MeetingTest_A,
		"ID.Clue.MeetingTest.A");
#endif
	UE_DEFINE_GAMEPLAY_TAG(ID_Claim, "ID.Claim");
	UE_DEFINE_GAMEPLAY_TAG(ID_Claim_FlowTest, "ID.Claim.FlowTest");
#if WITH_DEV_AUTOMATION_TESTS
	UE_DEFINE_GAMEPLAY_TAG(
		ID_Claim_MeetingTest_A,
		"ID.Claim.MeetingTest.A");
#endif
	UE_DEFINE_GAMEPLAY_TAG(ID_Element, "ID.Element");
	UE_DEFINE_GAMEPLAY_TAG(ID_Element_FlowTest, "ID.Element.FlowTest");
	UE_DEFINE_GAMEPLAY_TAG(ID_Element_CheckIn, "ID.Element.CheckIn");
	UE_DEFINE_GAMEPLAY_TAG(ID_Element_Maintenance, "ID.Element.Maintenance");
	UE_DEFINE_GAMEPLAY_TAG(ID_Element_Security, "ID.Element.Security");
	UE_DEFINE_GAMEPLAY_TAG(ID_Element_Guidance, "ID.Element.Guidance");

	#pragma endregion

	#pragma region Asset Mapping ID Tags

	UE_DEFINE_GAMEPLAY_TAG(ID_Icon, "ID.Icon");
	UE_DEFINE_GAMEPLAY_TAG(
		ID_Icon_Character_EditorImportTest,
		"ID.Icon.Character.EditorImportTest");

	#pragma endregion

	#pragma region Equipment Tags

	UE_DEFINE_GAMEPLAY_TAG(Equipment, "Equipment");
	UE_DEFINE_GAMEPLAY_TAG(Equipment_Slot, "Equipment.Slot");
	UE_DEFINE_GAMEPLAY_TAG(
		Equipment_Slot_Weapon,
		"Equipment.Slot.Weapon");
	UE_DEFINE_GAMEPLAY_TAG(
		Equipment_Slot_Head,
		"Equipment.Slot.Head");
	UE_DEFINE_GAMEPLAY_TAG(
		Equipment_Slot_Hands,
		"Equipment.Slot.Hands");
	UE_DEFINE_GAMEPLAY_TAG(
		Equipment_Slot_Feet,
		"Equipment.Slot.Feet");
	UE_DEFINE_GAMEPLAY_TAG(
		Equipment_Slot_UpperBody,
		"Equipment.Slot.UpperBody");

	#pragma endregion

	#pragma region Input Tags

	UE_DEFINE_GAMEPLAY_TAG(InputTag, "InputTag");

	#pragma endregion

	#pragma region UI Tags

	UE_DEFINE_GAMEPLAY_TAG(UI, "UI");
	UE_DEFINE_GAMEPLAY_TAG(UI_Game, "UI.Game");
	UE_DEFINE_GAMEPLAY_TAG(UI_Screen, "UI.Screen");
	UE_DEFINE_GAMEPLAY_TAG(UI_Screen_ClueSubmission, "UI.Screen.ClueSubmission");
	UE_DEFINE_GAMEPLAY_TAG(UI_Screen_PartyFormation, "UI.Screen.PartyFormation");
	UE_DEFINE_GAMEPLAY_TAG(UI_Screen_BattleResult, "UI.Screen.BattleResult");
	UE_DEFINE_GAMEPLAY_TAG(UI_Screen_Test, "UI.Screen.Test");
	UE_DEFINE_GAMEPLAY_TAG(UI_Modal, "UI.Modal");
	UE_DEFINE_GAMEPLAY_TAG(UI_Modal_DebugFlow, "UI.Modal.DebugFlow");
	UE_DEFINE_GAMEPLAY_TAG(UI_Modal_PhaseExitConfirm, "UI.Modal.PhaseExitConfirm");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer, "UI.Layer");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Game, "UI.Layer.Game");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Screen, "UI.Layer.Screen");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Modal, "UI.Layer.Modal");

	#pragma endregion

	#pragma region Ability System Tags

	UE_DEFINE_GAMEPLAY_TAG(Ability, "Ability");
	UE_DEFINE_GAMEPLAY_TAG(Ability_attack, "Ability.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Effect, "Effect");
	UE_DEFINE_GAMEPLAY_TAG(State, "State");
	UE_DEFINE_GAMEPLAY_TAG(State_Death, "State.Death");
	UE_DEFINE_GAMEPLAY_TAG(
		State_Death_Pending,
		"State.Death.Pending");

	#pragma endregion

	#pragma region Gameplay Message Channels

	UE_DEFINE_GAMEPLAY_TAG(Message, "Message");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Flow_Investigation_Completed,
		"Message.Flow.Investigation.Completed");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Flow_Investigation_Skipped,
		"Message.Flow.Investigation.Skipped");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Flow_Investigation_Ready,
		"Message.Flow.Investigation.Ready");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Flow_Meeting_Completed,
		"Message.Flow.Meeting.Completed");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Flow_PartyFormation_Entered,
		"Message.Flow.PartyFormation.Entered");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Flow_Battle_Ready,
		"Message.Flow.Battle.Ready");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Flow_Battle_Completed,
		"Message.Flow.Battle.Completed");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Battle_Participants_Initialized,
		"Message.Battle.Participants.Initialized");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Battle_WeaknessSlots_Initialized,
		"Message.Battle.WeaknessSlots.Initialized");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Battle_ActionState,
		"Message.Battle.ActionState");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Battle_ActionState_ActionSelection,
		"Message.Battle.ActionState.ActionSelection");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Battle_ActionState_TargetSelection,
		"Message.Battle.ActionState.TargetSelection");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Battle_ActionState_Processing,
		"Message.Battle.ActionState.Processing");
	UE_DEFINE_GAMEPLAY_TAG(
		Message_Battle_ActionState_Cleared,
		"Message.Battle.ActionState.Cleared");

	#pragma endregion

	#pragma region Battle Tags

	UE_DEFINE_GAMEPLAY_TAG(Action, "Action");
	UE_DEFINE_GAMEPLAY_TAG(Action_Ability, "Action.Ability");
	UE_DEFINE_GAMEPLAY_TAG(
		Action_Ability_BasicAttack,
		"Action.Ability.BasicAttack");
	UE_DEFINE_GAMEPLAY_TAG(
		Action_Ability_BasicSkill,
		"Action.Ability.BasicSkill");
	UE_DEFINE_GAMEPLAY_TAG(
		Action_Ability_Ultimate,
		"Action.Ability.Ultimate");
	UE_DEFINE_GAMEPLAY_TAG(
		Action_Ability_Counter,
		"Action.Ability.Counter");
	UE_DEFINE_GAMEPLAY_TAG(Event, "Event");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat, "Event.Combat");
	UE_DEFINE_GAMEPLAY_TAG(
		Event_Combat_BasicAttack_Hit,
		"Event.Combat.BasicAttack.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Battle, "Battle");

	#pragma endregion
}
