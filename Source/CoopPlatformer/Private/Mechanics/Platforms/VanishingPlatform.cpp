// Copyright Ricky Antonelli


#include "Mechanics/Platforms/VanishingPlatform.h"

// Sets default values
AVanishingPlatform::AVanishingPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Platform = CreateDefaultSubobject<UBoxComponent>(TEXT("Platform"));
	Platform->SetupAttachment(RootComp);
	Platform->SetIsReplicated(true);
	Platform->SetNotifyRigidBodyCollision(true);

	Sprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	Sprite->SetupAttachment(Platform);
	Sprite->SetIsReplicated(true);
	Sprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AVanishingPlatform::BeginPlay()
{
	Super::BeginPlay();

	Platform->OnComponentHit.AddDynamic(this, &AVanishingPlatform::OnBoxCollision);
	
}

void AVanishingPlatform::OnBoxCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor->ActorHasTag("Player") && !bIsVanishing)
	{
		// start the vanish
		bIsVanishing = true;
		FTimerHandle VanishTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(VanishTimerHandle, [&]() { StartVanish();  }, VanishingTime, false);
	}
}

void AVanishingPlatform::StartVanish()
{
	if (!Platform || !Sprite)
	{
		return;
	}
	Platform->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Sprite->SetVisibility(false);

	bIsVanishing = false;

	FTimerHandle ResetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(ResetTimerHandle, [&]() { ResetVanish();  }, RespawnTime, false);
}

void AVanishingPlatform::ResetVanish()
{
	if (!Platform || !Sprite)
	{
		return;
	}
	Platform->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Sprite->SetVisibility(true);
}
