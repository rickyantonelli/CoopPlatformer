// Copyright Ricky Antonelli


#include "Mechanics/Keys/TokenUnlockActor.h"

// Sets default values
ATokenUnlockActor::ATokenUnlockActor()
{	
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComp);
	Box->SetIsReplicated(true);

	Sprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	Sprite->SetupAttachment(Box);
	Sprite->SetIsReplicated(true);
	Sprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ATokenUnlockActor::BeginPlay()
{
	Super::BeginPlay();
	
	Box->OnComponentBeginOverlap.AddDynamic(this, &ATokenUnlockActor::OnBoxCollision);
}

// Called every frame
void ATokenUnlockActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && bCollected && CollectingCharacter->bDead)
	{
		bCollected = false;
		MulticastResetToken();
	}
	else if (HasAuthority() && bCollected && CollectingCharacter->GetCharacterMovement()->IsMovingOnGround())
	{
		MulticastUnlockActors();
	}
}

void ATokenUnlockActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor->ActorHasTag("Player"))
	{
		return;
	}

	// if this doesnt require player landing, simple unlock 
	if (!bLandedToken)
	{
		MulticastUnlockActors();
		MulticastHideToken();
	}
	else
	{
		bCollected = true;
		CollectingCharacter = Cast<AMyPaperCharacter>(OtherActor);
		MulticastHideToken();
	}
}

void ATokenUnlockActor::MulticastHideToken_Implementation()
{
	if (!Box || !Sprite)
	{
		return;
	}
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Sprite->SetVisibility(false);
}

void ATokenUnlockActor::MulticastResetToken_Implementation()
{
	if (!Box || !Sprite)
	{
		return;
	}
	Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Sprite->SetVisibility(true);
}

void ATokenUnlockActor::MulticastUnlockActors_Implementation()
{
	for (AActor* LockedActor : LockedActors)
	{
		UBoxComponent* LockBox = LockedActor->GetComponentByClass<UBoxComponent>();
		UPaperSpriteComponent* LockSprite = LockedActor->GetComponentByClass<UPaperSpriteComponent>();
		if (LockBox)
		{
			LockBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (LockSprite)
		{
			LockSprite->SetVisibility(false);
		}
	}

	bCollected = false;
	CollectingCharacter = nullptr;
}
