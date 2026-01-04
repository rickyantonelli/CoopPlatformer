// Copyright Ricky Antonelli


#include "Mechanics/Movement/SpringActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASpringActor::ASpringActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	LaunchPower = 1000;
	MovementDisableAmount = 0.1f;

}

// Called when the game starts or when spawned
void ASpringActor::BeginPlay()
{
	Super::BeginPlay();
	
	Box = GetComponentByClass<UBoxComponent>();
	if (Box)
	{
		Box->OnComponentBeginOverlap.AddDynamic(this, &ASpringActor::OnBoxCollision);
	}

	Sprite = GetComponentByClass<UPaperSpriteComponent>();
	if (Sprite)
	{
		Sprite->OnComponentBeginOverlap.AddDynamic(this, &ASpringActor::OnBoxCollision);
	}

	Flipbook = GetComponentByClass<UPaperFlipbookComponent>();

	if (Flipbook)
	{
		Flipbook->OnFinishedPlaying.AddDynamic(this, &ASpringActor::OnSpringFlipbookFinished);
	}
}

// Called every frame
void ASpringActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpringActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Player"))
	{
		AMyPaperCharacter* MyCharacter = Cast<AMyPaperCharacter>(OtherActor);
		if (MyCharacter)
		{
			MyCharacter->DisableJump(MovementDisableAmount);

			// reset player momentum before launching
			MyCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;

			FVector LaunchDirection = GetActorUpVector();
			MyCharacter->LaunchCharacter(LaunchDirection * LaunchPower, false, true);
		}

		if (Flipbook && Sprite)
		{
			Sprite->SetVisibility(false);
			Flipbook->SetVisibility(true);
			Flipbook->SetLooping(false);
			Flipbook->PlayFromStart();
		}
	}
}

void ASpringActor::OnSpringFlipbookFinished()
{
	if (Flipbook && Sprite)
	{
		Flipbook->SetVisibility(false);
		Sprite->SetVisibility(true);
	}
}

