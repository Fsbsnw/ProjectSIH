#include "PFBattleGameMode.h"

#include "Camera/CameraActor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Project_SIH/SIHGameplayTags.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleMessages.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleTypes.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFBattleLayoutProvider.h"
#include "Project_SIH/002_Systems/003_Combat/PFBattleRuntime.h"
#include "Project_SIH/002_Systems/003_Combat/PFBattleSpawner.h"
#include "Project_SIH/002_Systems/010_Map/PFMapContext.h"

APFBattleGameMode::APFBattleGameMode()
{
	DefaultPawnClass = nullptr;
}

EPFPhaseStartResult APFBattleGameMode::StartBattle(
	const FPFBattleEntryContext& Context)
{
	if (!Context.IsValid())
	{
		PF_LOG(TEXT("Battle context is invalid"));
		return EPFPhaseStartResult::InvalidContext;
	}

	if (!IsValid(m_BattleSpawner) || !IsValid(m_BattleRuntime))
	{
		PF_LOG(TEXT("Battle systems are not ready"));
		return EPFPhaseStartResult::NotReady;
	}

	// PF_LOG(
	// 	TEXT("[BattleEntrySmoke] CaseID=%s, Members=%d"),
	// 	*Context.m_CaseID.ToString(),
	// 	Context.m_Party.m_CharacterIDs.Num());
	//
	// for (const FGameplayTag& CharacterID :
	// 	Context.m_Party.m_CharacterIDs)
	// {
	// 	PF_LOG(
	// 		TEXT("[BattleEntrySmoke] Member=%s"),
	// 		*CharacterID.ToString());
	// }

	FPFBattleRuntimeContext RuntimeContext;

	const EPFBattlePreparationResult PreparationResult =
		m_BattleSpawner->PrepareBattle(
			Context,
			RuntimeContext);

	if (PreparationResult
		!= EPFBattlePreparationResult::Prepared)
	{
		PF_LOG(
			TEXT("Battle preparation failed. Result=%d"),
			static_cast<uint8>(PreparationResult));

		return EPFPhaseStartResult::NotReady;
	}

	if (m_RevealedWeaknessCountForDebug >= 0)
	{
		RuntimeContext.m_InitialWeaknessCounts.m_RevealedSlotCount =
			FMath::Clamp(
				m_RevealedWeaknessCountForDebug,
				0,
				FPFInitialWeaknessCounts::SlotCount);
	}

	return m_BattleRuntime->StartBattle(RuntimeContext);
}

bool APFBattleGameMode::SelectBattleAction(
	APFBattleCharacterBase* Requester,
	FGameplayTag ActionTag)
{
	if (!IsValid(m_BattleRuntime))
	{
		PF_LOG(TEXT("BattleRuntime is not ready"));
		return false;
	}

	return m_BattleRuntime->SelectBattleAction(Requester, ActionTag);
}

bool APFBattleGameMode::SelectBattleTarget(
	APFBattleCharacterBase* Requester,
	APFBattleCharacterBase* Target)
{
	if (!IsValid(m_BattleRuntime))
	{
		PF_LOG(TEXT("BattleRuntime is not ready"));
		return false;
	}

	return m_BattleRuntime->SelectBattleTarget(Requester, Target);
}

bool APFBattleGameMode::ConfirmBattleAction(
	APFBattleCharacterBase* Requester)
{
	if (!IsValid(m_BattleRuntime))
	{
		PF_LOG(TEXT("BattleRuntime is not ready"));
		return false;
	}

	return m_BattleRuntime->ConfirmBattleAction(Requester);
}

bool APFBattleGameMode::CancelBattleActionSelection(
	APFBattleCharacterBase* Requester)
{
	if (!IsValid(m_BattleRuntime))
	{
		PF_LOG(TEXT("BattleRuntime is not ready"));
		return false;
	}

	return m_BattleRuntime->CancelBattleActionSelection(Requester);
}

bool APFBattleGameMode::GetValidTargetsAndDefaultTarget(
	APFBattleCharacterBase* Requester,
	const FGameplayTag ActionTag,
	FPFBattleTargetSelection& OutSelection) const
{
	OutSelection = FPFBattleTargetSelection();
	if (!IsValid(m_BattleRuntime))
	{
		PF_LOG(TEXT("Target query failed: BattleRuntime is not ready"));
		return false;
	}

	return m_BattleRuntime->GetValidTargetsAndDefaultTarget(
		Requester, ActionTag, OutSelection);
}

void APFBattleGameMode::InitGame(
	const FString& MapName,
	const FString& Options,
	FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// PF_LOG(
	// 	TEXT("Initializing world systems. GameMode=%s, World=%s"),
	// 	*GetNameSafe(this),
	// 	*GetNameSafe(GetWorld()));

	m_BattleSpawner = NewObject<UPFBattleSpawner>(this);
	if (!IsValid(m_BattleSpawner))
	{
		PF_LOG(
			TEXT("Failed to create BattleSpawner. Outer=%s"),
			*GetNameSafe(this));
	}

	m_BattleRuntime = NewObject<UPFBattleRuntime>(this);
	if (IsValid(m_BattleRuntime))
	{
		m_BattleRuntime->Init();
	}
	else
	{
		PF_LOG(
			TEXT("Failed to create BattleRuntime. Outer=%s"),
			*GetNameSafe(this));
	}

	// PF_LOG(
	// 	TEXT("World system initialization complete. "
	// 		 "BattleSpawner=%s, BattleRuntime=%s"),
	// 	*GetNameSafe(m_BattleSpawner),
	// 	*GetNameSafe(m_BattleRuntime));
}

void APFBattleGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(m_BattleSpawner))
	{
		PF_LOG(TEXT("BattleSpawner is not valid"));
		return;
	}

	FPFBattleSpawnLayout SpawnLayout;
	ACameraActor* BattleCamera = nullptr;

	if (!TryResolveBattleMapSetup(
		SpawnLayout,
		BattleCamera))
	{
		return;
	}

	APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(this, 0);

	if (!IsValid(PlayerController))
	{
		PF_LOG(TEXT("Battle PlayerController is not valid"));
		return;
	}

	PlayerController->SetViewTarget(BattleCamera);

	m_BattleSpawner->Init(SpawnLayout);

	SendBattleReadyMessage();
}

void APFBattleGameMode::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(m_BattleRuntime))
	{
		m_BattleRuntime->Deinit();
	}

	Super::EndPlay(EndPlayReason);
}

bool APFBattleGameMode::TryResolveBattleMapSetup(
	FPFBattleSpawnLayout& OutLayout,
	ACameraActor*& OutCamera) const
{
	OutLayout = FPFBattleSpawnLayout();
	OutCamera = nullptr;

	TArray<AActor*> FoundMapContextActors;

	UGameplayStatics::GetAllActorsOfClass(
		this,
		APFMapContext::StaticClass(),
		FoundMapContextActors);

	if (FoundMapContextActors.Num() != 1)
	{
		PF_LOG(
			TEXT(
				"Battle map requires exactly one "
				"MapContext. Found=%d"),
			FoundMapContextActors.Num());
		return false;
	}

	APFMapContext* MapContext =
		Cast<APFMapContext>(FoundMapContextActors[0]);

	if (!IsValid(MapContext))
	{
		PF_LOG(TEXT("Found MapContext is invalid"));
		return false;
	}

	const IPFBattleLayoutProvider* LayoutProvider =
		Cast<IPFBattleLayoutProvider>(MapContext);

	if (!LayoutProvider)
	{
		PF_LOG(
			TEXT(
				"MapContext does not provide a "
				"battle layout. MapContext=%s"),
			*GetNameSafe(MapContext));
		return false;
	}

	if (!LayoutProvider->TryGetBattleSpawnLayout(
		OutLayout))
	{
		PF_LOG(
			TEXT(
				"Failed to get battle spawn layout. "
				"MapContext=%s"),
			*GetNameSafe(MapContext));
		return false;
	}

	if (!LayoutProvider->TryGetBattleCamera(
		OutCamera))
	{
		PF_LOG(
			TEXT(
				"Failed to get battle camera. "
				"MapContext=%s"),
			*GetNameSafe(MapContext));
		return false;
	}

	return true;
}

void APFBattleGameMode::SendBattleReadyMessage()
{
	if (!IsValid(m_BattleSpawner) || !IsValid(m_BattleRuntime))
	{
		PF_LOG(TEXT("Battle systems are not ready"));
		return;
	}

	UGameplayMessageSubsystem& MessageSubsystem =
		UGameplayMessageSubsystem::Get(this);

	// PF_LOG(TEXT("[BattleTravelSmoke] Sending Battle Ready"));

	MessageSubsystem.BroadcastMessage(
		SIHGameplayTags::Message_Flow_Battle_Ready.GetTag(),
		FPFBattleReadyMessage{});
}
