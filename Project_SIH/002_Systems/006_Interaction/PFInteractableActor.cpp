#include "PFInteractableActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "PFInteractionComponent.h"

APFInteractableActor::APFInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	m_StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(m_StaticMesh);

	m_InteractionRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRange"));
	m_InteractionRange->SetupAttachment(m_StaticMesh);
	m_InteractionRange->InitSphereRadius(200.0f);
	m_InteractionRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	m_InteractionRange->SetCollisionObjectType(ECC_WorldDynamic);
	m_InteractionRange->SetCollisionResponseToAllChannels(ECR_Ignore);
	m_InteractionRange->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	m_InteractionRange->SetGenerateOverlapEvents(true);

	m_InteractionRange->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APFInteractableActor::HandleInteractionRangeBeginOverlap);
	m_InteractionRange->OnComponentEndOverlap.AddUniqueDynamic(
		this,
		&APFInteractableActor::HandleInteractionRangeEndOverlap);
}

bool APFInteractableActor::Interact(APlayerController* Interactor)
{
	return false;
}

void APFInteractableActor::HandleInteractionRangeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	APlayerController* PlayerController =
		IsValid(PlayerPawn) ? Cast<APlayerController>(PlayerPawn->GetController()) : nullptr;
	UPFInteractionComponent* InteractionComponent =
		IsValid(PlayerController)
			? PlayerController->FindComponentByClass<UPFInteractionComponent>()
			: nullptr;

	if (IsValid(InteractionComponent))
	{
		TScriptInterface<IPFInteractableInterface> Interactable;
		Interactable.SetObject(this);
		Interactable.SetInterface(this);
		InteractionComponent->TryRegisterInteractable(Interactable);
	}
}

void APFInteractableActor::HandleInteractionRangeEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex)
{
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	APlayerController* PlayerController =
		IsValid(PlayerPawn) ? Cast<APlayerController>(PlayerPawn->GetController()) : nullptr;
	UPFInteractionComponent* InteractionComponent =
		IsValid(PlayerController)
			? PlayerController->FindComponentByClass<UPFInteractionComponent>()
			: nullptr;

	if (IsValid(InteractionComponent))
	{
		TScriptInterface<IPFInteractableInterface> Interactable;
		Interactable.SetObject(this);
		Interactable.SetInterface(this);
		InteractionComponent->UnregisterInteractable(Interactable);
	}
}
