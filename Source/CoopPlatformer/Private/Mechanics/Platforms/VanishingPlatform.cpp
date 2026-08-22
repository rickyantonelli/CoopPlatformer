// Copyright Ricky Antonelli

#include "Mechanics/Platforms/VanishingPlatform.h"
#include "Character/MyPaperCharacter.h"
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

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	bool bAnyOverlap = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Sprite->GetComponentLocation(),
		Sprite->GetComponentQuat(),
		ECC_Pawn,
		Sprite->GetCollisionShape(),
		Params
	);


	for (const FOverlapResult& Result : Overlaps)
	{
		if (AMyPaperCharacter* PC = Cast<AMyPaperCharacter>(Result.GetActor()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Player still on platform, delaying respawn."));
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
