#include "PFBattleResultScreen.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleMessages.h"

void UPFBattleResultScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Exit->OnClicked().AddUObject(
		this, &ThisClass::HandleExitClicked);
}

void UPFBattleResultScreen::SetResult(const FPFBattleResult& Result)
{
	Text_CaseID->SetText(FText::FromString(Result.m_CaseID.ToString()));

	FText ResultText;
	switch (Result.m_ResultType)
	{
	case EPFBattleResult::NormalVictory:
		ResultText = NSLOCTEXT(
			"PFBattleResult", "NormalVictory", "일반 승리");
		break;

	case EPFBattleResult::ForcedEviction:
		ResultText = NSLOCTEXT(
			"PFBattleResult", "ForcedEviction", "강제퇴실");
		break;

	case EPFBattleResult::Defeat:
		ResultText = NSLOCTEXT(
			"PFBattleResult", "Defeat", "패배");
		break;

	default:
		break;
	}

	Text_Result->SetText(ResultText);
}

void UPFBattleResultScreen::HandleExitClicked()
{
	UKismetSystemLibrary::QuitGame(
		this,
		GetOwningPlayer(),
		EQuitPreference::Quit,
		false);
}
