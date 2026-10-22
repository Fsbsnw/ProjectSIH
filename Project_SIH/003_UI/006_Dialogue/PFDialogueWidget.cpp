#include "PFDialogueWidget.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Engine/World.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/001_Data/000_Definitions/PFCharacterDefinition.h"
#include "Project_SIH/001_Data/001_AssetManagement/PFAssetManager.h"
#include "Project_SIH/002_Systems/011_Dialogue/PFDialogueSubsystem.h"
#include "PFDialogueChoiceEntryWidget.h"

void UPFDialogueWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Button_Continue->OnClicked().AddUObject(
		this,
		&UPFDialogueWidget::HandleContinueClicked);
}

void UPFDialogueWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	BindDialogueEvents();
	RefreshFromCurrentState();
}

void UPFDialogueWidget::NativeOnDeactivated()
{
	UnbindDialogueEvents();
	ResetPresentation();
	Super::NativeOnDeactivated();
}

void UPFDialogueWidget::BindDialogueEvents()
{
	UWorld* World = GetWorld();
	UPFDialogueSubsystem* DialogueSubsystem =
		IsValid(World)
			? World->GetSubsystem<UPFDialogueSubsystem>()
			: nullptr;
	if (!IsValid(DialogueSubsystem))
	{
		PF_LOG(TEXT("Dialogue Subsystem is not available."));
		return;
	}

	m_DialogueSubsystem = DialogueSubsystem;
	DialogueSubsystem->GetLinePresentedDelegate().AddUniqueDynamic(
		this,
		&UPFDialogueWidget::HandleLinePresented);
	DialogueSubsystem->GetChoicesPresentedDelegate().AddUniqueDynamic(
		this,
		&UPFDialogueWidget::HandleChoicesPresented);
	if (m_bEnableActionTestControls)
	{
		DialogueSubsystem->GetActionRequestedDelegate().AddUniqueDynamic(
			this,
			&UPFDialogueWidget::HandleActionRequested);
	}
	DialogueSubsystem->GetCompletedDelegate().AddUniqueDynamic(
		this,
		&UPFDialogueWidget::HandleDialogueCompleted);
	DialogueSubsystem->GetFailedDelegate().AddUniqueDynamic(
		this,
		&UPFDialogueWidget::HandleDialogueFailed);
	DialogueSubsystem->GetCancelledDelegate().AddUniqueDynamic(
		this,
		&UPFDialogueWidget::HandleDialogueCancelled);
}

void UPFDialogueWidget::UnbindDialogueEvents()
{
	UPFDialogueSubsystem* DialogueSubsystem = m_DialogueSubsystem.Get();
	if (!IsValid(DialogueSubsystem))
	{
		m_DialogueSubsystem.Reset();
		return;
	}

	DialogueSubsystem->GetLinePresentedDelegate().RemoveDynamic(
		this,
		&UPFDialogueWidget::HandleLinePresented);
	DialogueSubsystem->GetChoicesPresentedDelegate().RemoveDynamic(
		this,
		&UPFDialogueWidget::HandleChoicesPresented);
	DialogueSubsystem->GetActionRequestedDelegate().RemoveDynamic(
		this,
		&UPFDialogueWidget::HandleActionRequested);
	DialogueSubsystem->GetCompletedDelegate().RemoveDynamic(
		this,
		&UPFDialogueWidget::HandleDialogueCompleted);
	DialogueSubsystem->GetFailedDelegate().RemoveDynamic(
		this,
		&UPFDialogueWidget::HandleDialogueFailed);
	DialogueSubsystem->GetCancelledDelegate().RemoveDynamic(
		this,
		&UPFDialogueWidget::HandleDialogueCancelled);
	m_DialogueSubsystem.Reset();
}

void UPFDialogueWidget::RefreshFromCurrentState()
{
	UPFDialogueSubsystem* DialogueSubsystem = m_DialogueSubsystem.Get();
	if (!IsValid(DialogueSubsystem))
	{
		ResetPresentation();
		return;
	}

	if (DialogueSubsystem->HasCurrentLine())
	{
		HandleLinePresented(
			DialogueSubsystem->GetCurrentLinePresentation());
		return;
	}

	if (DialogueSubsystem->HasCurrentChoices())
	{
		HandleChoicesPresented(
			DialogueSubsystem->GetCurrentChoicePresentation());
		return;
	}

	if (m_bEnableActionTestControls
		&& DialogueSubsystem->IsWaitingForAction())
	{
		HandleActionRequested(
			DialogueSubsystem->GetCurrentActionPresentation());
		return;
	}

	ResetPresentation();
}

void UPFDialogueWidget::ClearChoiceEntries()
{
	VerticalBox_Choices->ClearChildren();
	VerticalBox_Choices->SetVisibility(ESlateVisibility::Collapsed);
}

void UPFDialogueWidget::ResetPresentation()
{
	Text_Speaker->SetText(FText::GetEmpty());
	Text_Dialogue->SetText(FText::GetEmpty());
	Image_Portrait->SetBrushFromTexture(nullptr);
	Image_Portrait->SetVisibility(ESlateVisibility::Collapsed);
	Button_Continue->SetVisibility(ESlateVisibility::Collapsed);
	m_CurrentActionNodeID = NAME_None;
	ClearChoiceEntries();
}

void UPFDialogueWidget::HandleLinePresented(
	const FPFDialogueLinePresentation& Presentation)
{
	const FPFDialogueLine& Line = Presentation.m_Line;
	const UPFCharacterDefinition* CharacterDefinition =
		UPFAssetManager::Get().GetCharacterDefinition(Line.m_SpeakerID);

	if (IsValid(CharacterDefinition))
	{
		Text_Speaker->SetText(CharacterDefinition->GetDisplayName());

		const TSoftObjectPtr<UTexture2D>& Portrait =
			CharacterDefinition->GetIcon();
		if (!Portrait.IsNull())
		{
			Image_Portrait->SetBrushFromSoftTexture(Portrait);
			Image_Portrait->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Image_Portrait->SetBrushFromTexture(nullptr);
			Image_Portrait->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		Text_Speaker->SetText(
			FText::FromName(Line.m_SpeakerID.GetTagName()));
		Image_Portrait->SetBrushFromTexture(nullptr);
		Image_Portrait->SetVisibility(ESlateVisibility::Collapsed);
	}

	Text_Dialogue->SetText(Line.m_Text);
	m_CurrentActionNodeID = NAME_None;
	ClearChoiceEntries();
	Button_Continue->SetVisibility(ESlateVisibility::Visible);
}

void UPFDialogueWidget::HandleChoicesPresented(
	const FPFDialogueChoicePresentation& Presentation)
{
	Button_Continue->SetVisibility(ESlateVisibility::Collapsed);
	m_CurrentActionNodeID = NAME_None;
	ClearChoiceEntries();

	if (!m_ChoiceEntryClass)
	{
		PF_LOG(TEXT("Dialogue Choice Entry class is not configured."));
		return;
	}

	for (const FPFDialogueChoice& Choice : Presentation.m_Choices)
	{
		UPFDialogueChoiceEntryWidget* ChoiceEntry =
			CreateWidget<UPFDialogueChoiceEntryWidget>(
				GetOwningPlayer(),
				m_ChoiceEntryClass);
		if (!IsValid(ChoiceEntry))
		{
			PF_LOG(
				TEXT("Failed to create Dialogue Choice Entry. ChoiceID=%s"),
				*Choice.m_ChoiceID.ToString());
			continue;
		}

		ChoiceEntry->SetChoice(Choice);
		ChoiceEntry->OnChoiceClicked().AddUObject(
			this,
			&UPFDialogueWidget::HandleChoiceClicked);
		VerticalBox_Choices->AddChild(ChoiceEntry);
	}

	VerticalBox_Choices->SetVisibility(ESlateVisibility::Visible);
}

void UPFDialogueWidget::HandleActionRequested(
	const FPFDialogueActionPresentation& Presentation)
{
	ResetPresentation();
	m_CurrentActionNodeID = Presentation.m_NodeID;

	Text_Speaker->SetText(FText::FromString(TEXT("Action Test")));
	Text_Dialogue->SetText(
		FText::FromName(Presentation.m_Action.m_ActionID.GetTagName()));

	if (!m_ChoiceEntryClass)
	{
		PF_LOG(TEXT("Dialogue Choice Entry class is not configured."));
		return;
	}

	for (const FPFDialogueActionBranch& Branch
		: Presentation.m_Action.m_Branches)
	{
		UPFDialogueChoiceEntryWidget* ResultEntry =
			CreateWidget<UPFDialogueChoiceEntryWidget>(
				GetOwningPlayer(),
				m_ChoiceEntryClass);
		if (!IsValid(ResultEntry))
		{
			PF_LOG(
				TEXT("Failed to create Action Result Entry. ResultID=%s"),
				*Branch.m_ResultID.ToString());
			continue;
		}

		FPFDialogueChoice ResultChoice;
		ResultChoice.m_ChoiceID = Branch.m_ResultID;
		ResultChoice.m_Text = FText::FromName(Branch.m_ResultID);
		ResultChoice.m_NextNodeID = Branch.m_NextNodeID;

		ResultEntry->SetChoice(ResultChoice);
		ResultEntry->OnChoiceClicked().AddUObject(
			this,
			&UPFDialogueWidget::HandleActionResultClicked);
		VerticalBox_Choices->AddChild(ResultEntry);
	}

	VerticalBox_Choices->SetVisibility(ESlateVisibility::Visible);
}

void UPFDialogueWidget::HandleDialogueCompleted()
{
	DeactivateWidget();
}

void UPFDialogueWidget::HandleDialogueFailed()
{
	PF_LOG(TEXT("Dialogue failed while the Dialogue Widget was active."));
	DeactivateWidget();
}

void UPFDialogueWidget::HandleDialogueCancelled()
{
	DeactivateWidget();
}

void UPFDialogueWidget::HandleContinueClicked()
{
	UPFDialogueSubsystem* DialogueSubsystem = m_DialogueSubsystem.Get();
	if (!IsValid(DialogueSubsystem))
	{
		return;
	}

	const EPFDialogueInputResult Result =
		DialogueSubsystem->ContinueDialogue();
	if (Result != EPFDialogueInputResult::Accepted)
	{
		PF_LOG(
			TEXT("Continue Dialogue was rejected. Result=%d"),
			static_cast<uint8>(Result));
	}
}

void UPFDialogueWidget::HandleChoiceClicked(const FName ChoiceID)
{
	UPFDialogueSubsystem* DialogueSubsystem = m_DialogueSubsystem.Get();
	if (!IsValid(DialogueSubsystem))
	{
		return;
	}

	const EPFDialogueInputResult Result =
		DialogueSubsystem->SelectChoice(ChoiceID);
	if (Result != EPFDialogueInputResult::Accepted)
	{
		PF_LOG(
			TEXT(
				"Dialogue Choice was rejected. "
				"ChoiceID=%s, Result=%d"),
			*ChoiceID.ToString(),
			static_cast<uint8>(Result));
	}
}

void UPFDialogueWidget::HandleActionResultClicked(const FName ResultID)
{
	UPFDialogueSubsystem* DialogueSubsystem = m_DialogueSubsystem.Get();
	if (!IsValid(DialogueSubsystem))
	{
		return;
	}

	const EPFDialogueInputResult Result =
		DialogueSubsystem->CompleteAction(
			m_CurrentActionNodeID,
			ResultID);
	if (Result != EPFDialogueInputResult::Accepted)
	{
		PF_LOG(
			TEXT(
				"Dialogue Action Result was rejected. "
				"NodeID=%s, ResultID=%s, Result=%d"),
			*m_CurrentActionNodeID.ToString(),
			*ResultID.ToString(),
			static_cast<uint8>(Result));
	}
}
