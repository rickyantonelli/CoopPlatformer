// Copyright Ricky Antonelli

#include "Mechanics/Platforms/VanishingPlatform.h"
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

		float DefaultFPS = Flipbook->GetFlipbook()->GetFramesPerSecond();
		int32 NumFrames = Flipbook->GetFlipbook()->GetNumFrames();
		float PlayRateMultiplier = (NumFrames / VanishingTime) / DefaultFPS;
		Flipbook->SetPlayRate(PlayRateMultiplier);
	}

	
}

void AVanishingPlatform::OnBoxCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor->ActorHasTag("Player") && !bIsVanishing)
	{
		bIsVanishing = true;

		Sprite->SetVisibility(false);
		Flipbook->SetVisibility(true);
		Flipbook->SetLooping(false);
		Flipbook->PlayFromStart();
	}
}

void AVanishingPlatform::ResetVanish()
{
	
	if (Platform)
	{
		Platform->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		Sprite->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	Sprite->SetVisibility(true);
}

void AVanishingPlatform::OnVanishFlipbookFinished()
{
	if (Platform)
	{
		Platform->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		Sprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	Flipbook->SetVisibility(false);

	bIsVanishing = false;

	FTimerHandle ResetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(ResetTimerHandle, [&]() { ResetVanish();  }, RespawnTime, false);
}
