// Copyright Ricky Antonelli

#include "Mechanics/Platforms/VanishingPlatform.h"
#include "Character/MyPaperCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "PaperFlipbook.h"

// Sets default values
AVanishingPlatform::AVanishingPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);
}

// Called when the game starts or when spawned
void AVanishingPlatform::BeginPlay()
{
	Super::BeginPlay();

	Platform = GetComponentByClass<UBoxComponent>();
	if (Platform)
	{
		Platform->OnComponentHit.AddDynamic(this, &AVanishingPlatform::OnBoxCollision);
	}

	Sprite = GetComponentByClass<UPaperSpriteComponent>();
	if (Sprite)
	{
		Sprite->OnComponentHit.AddDynamic(this, &AVanishingPlatform::OnBoxCollision);
	}

	Flipbook = GetComponentByClass<UPaperFlipbookComponent>();
	if (Flipbook)
	{
		Flipbook->OnFinishedPlaying.AddDynamic(this, &AVanishingPlatform::OnVanishFlipbookFinished);

		if (!VanishAnimation)
		{
			VanishAnimation = Flipbook->GetFlipbook();
		}

		if (VanishAnimation)
		{
			Flipbook->SetFlipbook(VanishAnimation);
			SetFlipbookPlayRateForDuration(VanishingTime);
		}
	}

	
}

void AVanishingPlatform::OnBoxCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor->ActorHasTag("Player") && !bIsVanishing && !bIsRespawning)
	{
		bIsVanishing = true;

		Sprite->SetVisibility(false);
		if (VanishAnimation)
		{
			Flipbook->SetFlipbook(VanishAnimation);
			SetFlipbookPlayRateForDuration(VanishingTime);
		}
		Flipbook->SetVisibility(true);
		Flipbook->SetLooping(false);
		Flipbook->PlayFromStart();
	}
}

void AVanishingPlatform::ResetVanish()
{
	TArray<FOverlapResult> Overlaps;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	UPrimitiveComponent* PlatformCollision = Platform ? Cast<UPrimitiveComponent>(Platform) : Sprite;
	if (PlatformCollision)
	{
		const bool bAnyOverlap = GetWorld()->OverlapMultiByObjectType(
			Overlaps,
			PlatformCollision->GetComponentLocation(),
			PlatformCollision->GetComponentQuat(),
			ECC_Pawn,
			PlatformCollision->GetCollisionShape(),
			Params
		);

		if (bAnyOverlap && HasAuthority())
		{
			for (const FOverlapResult& Result : Overlaps)
			{
				if (AMyPaperCharacter* PC = Cast<AMyPaperCharacter>(Result.GetActor()))
				{
					RepositionPlayerIfStuck(PC);
				}
			}
		}
	}

	if (Platform)
	{
		Platform->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		Sprite->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	Sprite->SetVisibility(true);

	if (Flipbook)
	{
		Flipbook->SetVisibility(false);
		if (VanishAnimation)
		{
			Flipbook->SetFlipbook(VanishAnimation);
			SetFlipbookPlayRateForDuration(VanishingTime);
		}
	}

	bIsRespawning = false;
	bIsVanishing = false;
}

bool AVanishingPlatform::IsPlayerOverlappingPlatform(AActor* Player)
{
	if (!Player)
	{
		return false;
	}

	UCapsuleComponent* PlayerCapsule = Player->FindComponentByClass<UCapsuleComponent>();
	if (!PlayerCapsule)
	{
		return false;
	}

	UPrimitiveComponent* PlatformCollision = Platform ? Cast<UPrimitiveComponent>(Platform) : Sprite;
	if (!PlatformCollision)
	{
		return false;
	}

	const FVector PlayerLocation = Player->GetActorLocation();
	const float PlayerRadius = PlayerCapsule->GetScaledCapsuleRadius();
	const float PlayerHalfHeight = PlayerCapsule->GetScaledCapsuleHalfHeight();
	const FVector PlatformLocation = GetActorLocation();
	const FVector PlatformExtent = PlatformCollision->Bounds.BoxExtent;

	const bool bOverlapsX =
		(PlayerLocation.X - PlayerRadius < PlatformLocation.X + PlatformExtent.X) &&
		(PlayerLocation.X + PlayerRadius > PlatformLocation.X - PlatformExtent.X);
	const bool bOverlapsY =
		(PlayerLocation.Y - PlayerRadius < PlatformLocation.Y + PlatformExtent.Y) &&
		(PlayerLocation.Y + PlayerRadius > PlatformLocation.Y - PlatformExtent.Y);
	const bool bOverlapsZ =
		(PlayerLocation.Z - PlayerHalfHeight < PlatformLocation.Z + PlatformExtent.Z) &&
		(PlayerLocation.Z + PlayerHalfHeight > PlatformLocation.Z - PlatformExtent.Z);

	return bOverlapsX && bOverlapsY && bOverlapsZ;
}

void AVanishingPlatform::RepositionPlayerIfStuck(AActor* Player)
{
	if (!Player || !IsPlayerOverlappingPlatform(Player))
	{
		return;
	}

	UPrimitiveComponent* PlatformCollision = Platform ? Cast<UPrimitiveComponent>(Platform) : Sprite;
	if (!PlatformCollision)
	{
		return;
	}

	const FVector PlatformExtent = PlatformCollision->Bounds.BoxExtent;
	FVector NewLocation = Player->GetActorLocation();
	NewLocation.Z = GetActorLocation().Z + PlatformExtent.Z + PlayerPushDistance;

	Player->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

	UE_LOG(LogTemp, Warning, TEXT("Player %s was stuck in vanishing platform %s, pushed up on Z axis to %f"),
		*Player->GetName(), *GetName(), NewLocation.Z);
}

void AVanishingPlatform::OnVanishFlipbookFinished()
{
	if (bIsRespawning)
	{
		ResetVanish();
		return;
	}

	if (Platform)
	{
		Platform->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		Sprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (Flipbook && UnvanishAnimation && RespawnTime > KINDA_SMALL_NUMBER)
	{
		bIsRespawning = true;
		Flipbook->SetFlipbook(UnvanishAnimation);
		Flipbook->SetLooping(false);
		SetFlipbookPlayRateForDuration(RespawnTime);
		Flipbook->SetVisibility(true);
		Flipbook->PlayFromStart();
		return;
	}

	Flipbook->SetVisibility(false);
	bIsVanishing = false;

	FTimerHandle ResetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(ResetTimerHandle, [this]() { ResetVanish();  }, RespawnTime, false);
}

void AVanishingPlatform::SetFlipbookPlayRateForDuration(float Duration)
{
	if (!Flipbook || Duration <= KINDA_SMALL_NUMBER || !Flipbook->GetFlipbook())
	{
		return;
	}

	const float DefaultFPS = Flipbook->GetFlipbook()->GetFramesPerSecond();
	const int32 NumFrames = Flipbook->GetFlipbook()->GetNumFrames();
	if (DefaultFPS <= KINDA_SMALL_NUMBER || NumFrames <= 0)
	{
		return;
	}

	const float PlayRateMultiplier =
		(static_cast<float>(NumFrames) / Duration) / DefaultFPS;
	Flipbook->SetPlayRate(PlayRateMultiplier);
}
