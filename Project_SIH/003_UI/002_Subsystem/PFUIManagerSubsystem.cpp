#include "PFUIManagerSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Project_SIH/003_UI/000_Foundation/PFActivatableWidget.h"
#include "Project_SIH/003_UI/001_Config/PFUIConfig.h"
#include "Project_SIH/003_UI/001_Config/PFUISettings.h"
#include "Project_SIH/003_UI/PFPrimaryGameLayout.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/000_Core/003_Utilities/PFGameplayTagUtilities.h"
#include "Project_SIH/SIHGameplayTags.h"

namespace
{
	bool AreSameInputActions(
		const FDataTableRowHandle& Left,
		const FDataTableRowHandle& Right)
	{
		return Left.DataTable == Right.DataTable
			&& Left.RowName == Right.RowName;
	}

	bool ContainsInputAction(
		const TArray<FPFUIInputBinding>& Bindings,
		const FDataTableRowHandle& InputAction)
	{
		return Bindings.ContainsByPredicate(
			[&InputAction](const FPFUIInputBinding& Binding)
			{
				return AreSameInputActions(Binding.m_InputAction, InputAction);
			});
	}
}

void UPFUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UCommonUIActionRouterBase>();

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		return;
	}

	HandlePlayerControllerChanged(LocalPlayer->GetPlayerController(GetWorld()));
}

void UPFUIManagerSubsystem::Deinitialize()
{
	ReleasePrimaryGameLayout();
	m_GlobalUIConfig = nullptr;
	m_GameModeUIConfig = nullptr;
	Super::Deinitialize();
}

void UPFUIManagerSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);
	HandlePlayerControllerChanged(NewPlayerController);
}

UPFPrimaryGameLayout* UPFUIManagerSubsystem::GetPrimaryGameLayout() const
{
	return m_PrimaryGameLayout;
}

const UPFUIConfig* UPFUIManagerSubsystem::GetUIConfig() const
{
	return m_GameModeUIConfig;
}

void UPFUIManagerSubsystem::GetResolvedInputBindings(
	TArray<FPFUIInputBinding>& OutBindings) const
{
	OutBindings.Reset();

	TMap<FGameplayTag, FPFUIWidgetDefinition> GameModeDefinitions;
	if (IsValid(m_GameModeUIConfig))
	{
		m_GameModeUIConfig->GetWidgetDefinitions(GameModeDefinitions);
	}

	TMap<FGameplayTag, FPFUIWidgetDefinition> GlobalDefinitions;
	if (IsValid(m_GlobalUIConfig))
	{
		m_GlobalUIConfig->GetWidgetDefinitions(GlobalDefinitions);
	}

	TSet<FGameplayTag> GameModeWidgetIDs;
	for (const TPair<FGameplayTag, FPFUIWidgetDefinition>& Definition : GameModeDefinitions)
	{
		GameModeWidgetIDs.Add(Definition.Key);
	}

	auto AppendBindings = [&OutBindings](
		const TMap<FGameplayTag, FPFUIWidgetDefinition>& Definitions,
		const TSet<FGameplayTag>* OverriddenWidgetIDs)
	{
		TArray<FGameplayTag> WidgetIDs;
		Definitions.GetKeys(WidgetIDs);
		WidgetIDs.Sort(
			[](const FGameplayTag& Left, const FGameplayTag& Right)
			{
				return Left.ToString() < Right.ToString();
			});

		for (const FGameplayTag& WidgetID : WidgetIDs)
		{
			if (OverriddenWidgetIDs != nullptr && OverriddenWidgetIDs->Contains(WidgetID))
			{
				continue;
			}

			const FPFUIWidgetDefinition* Definition = Definitions.Find(WidgetID);
			if (Definition == nullptr || Definition->m_OpenInputAction.IsNull())
			{
				continue;
			}

			if (ContainsInputAction(OutBindings, Definition->m_OpenInputAction))
			{
				PF_LOG(
					TEXT("UI input action is assigned to more than one Widget. WidgetID=%s, Row=%s"),
					*WidgetID.ToString(),
					*Definition->m_OpenInputAction.RowName.ToString());
				continue;
			}

			FPFUIInputBinding& Binding = OutBindings.AddDefaulted_GetRef();
			Binding.m_WidgetID = WidgetID;
			Binding.m_InputAction = Definition->m_OpenInputAction;
		}
	};

	// GameMode Config가 동일 Widget ID와 동일 Input Action 모두에서 공용 Config보다 우선합니다.
	AppendBindings(GameModeDefinitions, nullptr);
	AppendBindings(GlobalDefinitions, &GameModeWidgetIDs);
}

UCommonActivatableWidget* UPFUIManagerSubsystem::ShowHUDLayout()
{
	if (!IsValid(m_PrimaryGameLayout) || !IsValid(m_GameModeUIConfig))
	{
		return nullptr;
	}

	UClass* HUDClass = m_GameModeUIConfig->GetHUDLayOutClass().LoadSynchronous();
	if (!IsValid(HUDClass))
	{
		PF_LOG(TEXT("HUD class is not configured in the GameMode UI Config."));
		return nullptr;
	}

	// HUD는 GameStack의 기반 화면입니다. 일반 Game Widget은 이후 이 HUD 위에 쌓입니다.
	m_PrimaryGameLayout->ClearLayer(SIHGameplayTags::UI_Layer_Game.GetTag());
	return m_PrimaryGameLayout->PushWidgetToLayer(
		SIHGameplayTags::UI_Layer_Game.GetTag(),
		HUDClass);
}

UCommonActivatableWidget* UPFUIManagerSubsystem::ShowWidget(FGameplayTag WidgetID)
{
	if (PFGameplayTagUtilities::IsValidChildTag(WidgetID, SIHGameplayTags::UI_Game.GetTag()))
	{
		return ShowGameWidget(WidgetID);
	}
	if (PFGameplayTagUtilities::IsValidChildTag(WidgetID, SIHGameplayTags::UI_Screen.GetTag()))
	{
		return ShowScreenWidget(WidgetID);
	}
	if (PFGameplayTagUtilities::IsValidChildTag(WidgetID, SIHGameplayTags::UI_Modal.GetTag()))
	{
		return ShowModalWidget(WidgetID);
	}

	PF_LOG(TEXT("Widget ID is outside the UI route domains. WidgetID=%s"), *WidgetID.ToString());
	return nullptr;
}

UCommonActivatableWidget* UPFUIManagerSubsystem::ShowGameWidget(const FGameplayTag& WidgetID)
{
	if (!IsValid(m_PrimaryGameLayout))
	{
		return nullptr;
	}

	// 현재 게임 모드 전용 UI 설정에서 먼저 위젯을 찾습니다.
	const FPFUIWidgetDefinition* WidgetDefinition = IsValid(m_GameModeUIConfig)
		? m_GameModeUIConfig->FindGameWidgetDefinition(WidgetID)
		: nullptr;

	// 게임 모드 전용 위젯이 없으면 공용 UI 설정에서 다시 찾습니다.
	if (WidgetDefinition == nullptr && IsValid(m_GlobalUIConfig))
	{
		WidgetDefinition = m_GlobalUIConfig->FindGameWidgetDefinition(WidgetID);
	}

	const TSubclassOf<UCommonActivatableWidget> WidgetClass = LoadWidgetClass(
		WidgetDefinition,
		WidgetID);
	return WidgetClass
		? m_PrimaryGameLayout->PushWidgetToLayer(
			SIHGameplayTags::UI_Layer_Game.GetTag(),
			WidgetClass)
		: nullptr;
}

UCommonActivatableWidget* UPFUIManagerSubsystem::ShowScreenWidget(const FGameplayTag& WidgetID)
{
	if (!IsValid(m_PrimaryGameLayout))
	{
		return nullptr;
	}

	// 현재 게임 모드 전용 UI 설정에서 먼저 위젯을 찾습니다.
	const FPFUIWidgetDefinition* WidgetDefinition = IsValid(m_GameModeUIConfig)
		? m_GameModeUIConfig->FindScreenWidgetDefinition(WidgetID)
		: nullptr;

	// 게임 모드 전용 위젯이 없으면 공용 UI 설정에서 다시 찾습니다.
	if (WidgetDefinition == nullptr && IsValid(m_GlobalUIConfig))
	{
		WidgetDefinition = m_GlobalUIConfig->FindScreenWidgetDefinition(WidgetID);
	}

	const TSubclassOf<UCommonActivatableWidget> WidgetClass = LoadWidgetClass(
		WidgetDefinition,
		WidgetID);
	return WidgetClass
		? m_PrimaryGameLayout->PushWidgetToLayer(
			SIHGameplayTags::UI_Layer_Screen.GetTag(),
			WidgetClass)
		: nullptr;
}

UCommonActivatableWidget* UPFUIManagerSubsystem::ShowModalWidget(const FGameplayTag& WidgetID)
{
	if (!IsValid(m_PrimaryGameLayout))
	{
		return nullptr;
	}

	// 현재 게임 모드 전용 UI 설정에서 먼저 위젯을 찾습니다.
	const FPFUIWidgetDefinition* WidgetDefinition = IsValid(m_GameModeUIConfig)
		? m_GameModeUIConfig->FindModalWidgetDefinition(WidgetID)
		: nullptr;

	// 게임 모드 전용 위젯이 없으면 공용 UI 설정에서 다시 찾습니다.
	if (WidgetDefinition == nullptr && IsValid(m_GlobalUIConfig))
	{
		WidgetDefinition = m_GlobalUIConfig->FindModalWidgetDefinition(WidgetID);
	}

	const TSubclassOf<UCommonActivatableWidget> WidgetClass = LoadWidgetClass(
		WidgetDefinition,
		WidgetID);
	return WidgetClass
		? m_PrimaryGameLayout->PushWidgetToLayer(
			SIHGameplayTags::UI_Layer_Modal.GetTag(),
			WidgetClass)
		: nullptr;
}

void UPFUIManagerSubsystem::HandlePlayerControllerChanged(APlayerController* PlayerController)
{
	if (!IsValid(PlayerController) || !PlayerController->IsLocalPlayerController())
	{
		return;
	}

	UWorld* PlayerWorld = PlayerController->GetWorld();
	if (IsValid(m_PrimaryGameLayout) && m_LayoutWorld.Get() != PlayerWorld)
	{
		ReleasePrimaryGameLayout();
	}

	const bool bConfigChanged = LoadUIConfigsForGameMode(PlayerController);

	if (IsValid(m_PrimaryGameLayout))
	{
		m_PrimaryGameLayout->SetOwningPlayer(PlayerController);
		if (!m_PrimaryGameLayout->IsInViewport())
		{
			m_PrimaryGameLayout->AddToPlayerScreen();
		}
		if (bConfigChanged)
		{
			ShowHUDLayout();
		}
		return;
	}

	const UPFUISettings* UISettings = GetDefault<UPFUISettings>();
	TSubclassOf<UPFPrimaryGameLayout> LayoutClass =
		UISettings->GetPrimaryGameLayoutClass().LoadSynchronous();
	if (!LayoutClass)
	{
		PF_LOG(TEXT("PrimaryGameLayout class is not configured."));
		return;
	}

	UPFPrimaryGameLayout* NewLayout = CreateWidget<UPFPrimaryGameLayout>(PlayerController, LayoutClass);
	if (!IsValid(NewLayout))
	{
		PF_LOG(TEXT("Failed to create PrimaryGameLayout."));
		return;
	}

	if (!NewLayout->AddToPlayerScreen())
	{
		PF_LOG(TEXT("Failed to add PrimaryGameLayout to the player screen."));
		return;
	}

	m_PrimaryGameLayout = NewLayout;
	m_LayoutWorld = PlayerWorld;
	ShowHUDLayout();
}

void UPFUIManagerSubsystem::ReleasePrimaryGameLayout()
{
	if (IsValid(m_PrimaryGameLayout))
	{
		m_PrimaryGameLayout->RemoveFromParent();
	}

	m_PrimaryGameLayout = nullptr;
	m_LayoutWorld.Reset();
}

bool UPFUIManagerSubsystem::LoadUIConfigsForGameMode(const APlayerController* PlayerController)
{
	const UWorld* World = IsValid(PlayerController) ? PlayerController->GetWorld() : nullptr;
	const AGameModeBase* GameMode = IsValid(World) ? World->GetAuthGameMode() : nullptr;
	if (!IsValid(GameMode))
	{
		PF_LOG(TEXT("GameMode is not available for UI Config selection."));
		return false;
	}

	const UPFUISettings* UISettings = GetDefault<UPFUISettings>();
	UPFUIConfig* NewGlobalUIConfig = UISettings->GetGlobalUIConfig().LoadSynchronous();
	const TSoftObjectPtr<UPFUIConfig>* GameModeUIConfigAsset =
		UISettings->FindUIConfigForGameMode(GameMode->GetClass());
	UPFUIConfig* NewGameModeUIConfig = GameModeUIConfigAsset != nullptr
		? GameModeUIConfigAsset->LoadSynchronous()
		: nullptr;
	if (!IsValid(NewGameModeUIConfig))
	{
		PF_LOG(
			TEXT("UI Config is not configured for GameMode. GameMode=%s"),
			*GameMode->GetClass()->GetName());
	}

	if (NewGlobalUIConfig == m_GlobalUIConfig && NewGameModeUIConfig == m_GameModeUIConfig)
	{
		return false;
	}

	if (IsValid(m_PrimaryGameLayout))
	{
		m_PrimaryGameLayout->ClearAllWidgets();
	}
	m_GlobalUIConfig = NewGlobalUIConfig;
	m_GameModeUIConfig = NewGameModeUIConfig;

	return true;
}

TSubclassOf<UCommonActivatableWidget> UPFUIManagerSubsystem::LoadWidgetClass(
	const FPFUIWidgetDefinition* WidgetDefinition,
	const FGameplayTag& WidgetID) const
{
	if (WidgetDefinition == nullptr || WidgetDefinition->m_WidgetClass.IsNull())
	{
		PF_LOG(TEXT("Widget is not configured. WidgetID=%s"), *WidgetID.ToString());
		return nullptr;
	}

	UClass* LoadedClass = WidgetDefinition->m_WidgetClass.LoadSynchronous();
	if (!IsValid(LoadedClass))
	{
		PF_LOG(TEXT("Failed to load Widget class. WidgetID=%s"), *WidgetID.ToString());
		return nullptr;
	}

	return LoadedClass;
}
