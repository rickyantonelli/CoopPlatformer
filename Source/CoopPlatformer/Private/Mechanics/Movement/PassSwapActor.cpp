// Copyright Ricky Antonelli

#include "Mechanics/Movement/PassSwapActor.h"
#include "Character/MyPaperCharacter.h"
#include "Systems/CoopPlatformerGameModeBase.h"
#include "PaperSpriteComponent.h"
#include "Components/CapsuleComponent.h"
#include <Kismet/GameplayStatics.h>

APassSwapActor::APassSwapActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	PlayerPushDistance = 20.0f;
}

void APassSwapActor::BeginPlay()
{
	Super::BeginPlay();

	// Hide all actors in the second set
	for (AActor* Actor : SecondSetActors)
	{
		UBoxComponent* LockBox = Actor->GetComponentByClass<UBoxComponent>();
		UPaperSpriteComponent* LockSprite = Actor->GetComponentByClass<UPaperSpriteComponent>();

		if (LockBox)
		{
			LockBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (LockSprite && OffSprite)
		{
			LockSprite->SetSprite(OffSprite);
			if (!LockBox)
			{
				LockSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}

	if (ActivationArea)
	{
		ActivationArea->OnActorBeginOverlap.AddDynamic(this, &APassSwapActor::OnActivateTriggerBeginOverlap);
		ActivationArea->OnActorEndOverlap.AddDynamic(this, &APassSwapActor::OnActivateTriggerEndOverlap);
	}

	MyGameStateCoop = GetWorld()->GetGameState<AMyGameStateBase>();
}

void APassSwapActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;

	if (!BindingsSet)
	{
		TArray<AActor*> Controllers;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), Controllers);

		if (Controllers.Num() == 2)
		{
			for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
			{
				AController2D* PC = Cast<AController2D>(*Iterator);
				if (PC)
				{
					PC->OnPassActivated.AddDynamic(this, &APassSwapActor::OnPassActivated);
				}
			}
			BindingsSet = true;
		}
	}
}

bool APassSwapActor::IsPlayerOverlappingYAxis(AActor* Player, AActor* Platform)
{
	if (!Player || !Platform) return false;

	UCapsuleComponent* PlayerCapsule = Player->FindComponentByClass<UCapsuleComponent>();
	if (!PlayerCapsule) return false;

	// TODO: convert to just sprite components later
	UBoxComponent* PlatformBox = Platform->GetComponentByClass<UBoxComponent>();
	UPrimitiveComponent* PlatformCollision = PlatformBox ? Cast<UPrimitiveComponent>(PlatformBox) : Platform->GetComponentByClass<UPaperSpriteComponent>();

	if (!PlatformCollision) return false;

	// Get the bounds of the player and platform
	// then check for overlap on all three axes
	// TODO: probably overkill, I think we can simplify this long term
	FVector PlayerLocation = Player->GetActorLocation();
	float PlayerRadius = PlayerCapsule->GetScaledCapsuleRadius();

	FVector PlatformLocation = Platform->GetActorLocation();
	FVector PlatformExtent = PlatformCollision->Bounds.BoxExtent;
	float PlayerYMin = PlayerLocation.Y - PlayerRadius;
	float PlayerYMax = PlayerLocation.Y + PlayerRadius;
	float PlatformYMin = PlatformLocation.Y - PlatformExtent.Y;
	float PlatformYMax = PlatformLocation.Y + PlatformExtent.Y;

	bool bOverlapsY = (PlayerYMin < PlatformYMax) && (PlayerYMax > PlatformYMin);

	float PlayerHalfHeight = PlayerCapsule->GetScaledCapsuleHalfHeight();

	float PlayerXMin = PlayerLocation.X - PlayerRadius;
	float PlayerXMax = PlayerLocation.X + PlayerRadius;
	float PlatformXMin = PlatformLocation.X - PlatformExtent.X;
	float PlatformXMax = PlatformLocation.X + PlatformExtent.X;

	float PlayerZMin = PlayerLocation.Z - PlayerHalfHeight;
	float PlayerZMax = PlayerLocation.Z + PlayerHalfHeight;
	float PlatformZMin = PlatformLocation.Z - PlatformExtent.Z;
	float PlatformZMax = PlatformLocation.Z + PlatformExtent.Z;

	bool bOverlapsX = (PlayerXMin < PlatformXMax) && (PlayerXMax > PlatformXMin);
	bool bOverlapsZ = (PlayerZMin < PlatformZMax) && (PlayerZMax > PlatformZMin);

	return bOverlapsY && bOverlapsX && bOverlapsZ;
}

void APassSwapActor::RepositionPlayerIfStuck(AActor* Player, AActor* Platform)
{
	if (!Player || !Platform) return;

	// validate if the player is actually stuck, then perform a slight push up
	if (IsPlayerOverlappingYAxis(Player, Platform))
	{
		// TODO: we're doing this twice, just pass it through from IsPlayerOverlappingYAxis
		UBoxComponent* PlatformBox = Platform->GetComponentByClass<UBoxComponent>();
		UPrimitiveComponent* PlatformCollision = PlatformBox ? Cast<UPrimitiveComponent>(PlatformBox) : Platform->GetComponentByClass<UPaperSpriteComponent>();

		if (!PlatformCollision) return;

		FVector PlatformExtent = PlatformCollision->Bounds.BoxExtent;

		FVector NewLocation = Player->GetActorLocation();
		NewLocation.Z = Platform->GetActorLocation().Z + PlatformExtent.Z + PlayerPushDistance;

		Player->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

		UE_LOG(LogTemp, Warning, TEXT("Player %s was stuck in platform %s, pushed up on Z axis to %f"),
			*Player->GetName(), *Platform->GetName(), NewLocation.Z);
	}
}

void APassSwapActor::MulticastSwapActors_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("MulticastSwapActors called. FirstSetActive: %s"), FirstSetActive ? TEXT("true") : TEXT("false"));

	const TArray<AActor*>& ActivatingActors = FirstSetActive ? SecondSetActors : FirstSetActors;
	const TArray<AActor*>& DisablingActors = FirstSetActive ? FirstSetActors : SecondSetActors;

	UE_LOG(LogTemp, Log, TEXT("Swapping actors"));

	// Get all player characters in the world
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Player"), Players);

	// FIRST: Check for stuck players BEFORE activating platforms
	for (AActor* ActivatingActor : ActivatingActors)
	{
		for (AActor* Player : Players)
		{
			RepositionPlayerIfStuck(Player, ActivatingActor);
		}
	}

	// THEN: Activate the platforms
	for (AActor* ActivatingActor : ActivatingActors)
	{
		UBoxComponent* LockBox = ActivatingActor->GetComponentByClass<UBoxComponent>();
		UPaperSpriteComponent* LockSprite = ActivatingActor->GetComponentByClass<UPaperSpriteComponent>();

		if (LockBox)
		{
			LockBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}

		if (LockSprite && OnSprite)
		{
			LockSprite->SetSprite(OnSprite);
			if (!LockBox)
			{
				LockSprite->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}
		}
	}

	// Disable the other set
	for (AActor* DisablingActor : DisablingActors)
	{
		UBoxComponent* LockBox = DisablingActor->GetComponentByClass<UBoxComponent>();
		UPaperSpriteComponent* LockSprite = DisablingActor->GetComponentByClass<UPaperSpriteComponent>();

		if (LockBox)
		{
			LockBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (LockSprite && OffSprite)
		{
			LockSprite->SetSprite(OffSprite);
			if (!LockBox)
			{
				LockSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}

	FirstSetActive = !FirstSetActive;
}

void APassSwapActor::OnPassActivated()
{
	if (bActivated)
	{
		MulticastSwapActors();
	}
}

void APassSwapActor::OnActivateTriggerBeginOverlap(AActor* PlayerActor, AActor* OtherActor)
{
	if (!HasAuthority()) return;
	if (!OtherActor->ActorHasTag("Player")) return;

	if (!CurrentActiveActors.Contains(OtherActor))
	{
		CurrentActiveActors.Add(OtherActor);
	}

	if (CurrentActiveActors.Num() == 2)
	{
		bActivated = true;
	}
}

void APassSwapActor::OnActivateTriggerEndOverlap(AActor* PlayerActor, AActor* OtherActor)
{
	if (!HasAuthority()) return;
	if (!OtherActor->ActorHasTag("Player")) return;

	if (CurrentActiveActors.Contains(OtherActor))
	{
		CurrentActiveActors.Remove(OtherActor);
	}

	if (CurrentActiveActors.Num() < 2)
	{
		bActivated = false;
	}
}

void APassSwapActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APassSwapActor, bActivated);
	DOREPLIFETIME(APassSwapActor, CurrentActiveActors);
}