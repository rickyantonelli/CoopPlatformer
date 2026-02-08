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

	Flipbook = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Flipbook"));
	Flipbook->SetupAttachment(RootComp);
	Flipbook->SetIsReplicated(true);
	Flipbook->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	if (!Box || !Flipbook)
	{
		return;
	}
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Flipbook->SetVisibility(false);
}

void ATokenUnlockActor::MulticastResetToken_Implementation()
{
	if (!Box || !Flipbook)
	{
		return;
	}
	Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Flipbook->SetVisibility(true);
}

void ATokenUnlockActor::MulticastUnlockActors_Implementation()
{
	for (AActor* LockedActor : LockedActors)
	{
		// for a while the implementation will be as such - to keep flexibility
		// first check if there is a static mesh, if there is then just set the visibility of the mesh and collision off
		// if there is no mesh, then we are dealing with a sprite - which is going to have a box component and PaperSprite component


		// set this to false so that we dont check overlaps anymore since it's already been unlocked
		UStaticMeshComponent* LockMesh = LockedActor->GetComponentByClass<UStaticMeshComponent>();
		if (LockMesh)
		{
			LockMesh->SetVisibility(false);
			LockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			UBoxComponent* LockBox = LockedActor->GetComponentByClass<UBoxComponent>();
			UPaperSpriteComponent* LockSprite = LockedActor->GetComponentByClass<UPaperSpriteComponent>();
			UPaperFlipbookComponent* FlipbookComp = LockedActor->GetComponentByClass<UPaperFlipbookComponent>();
			if (LockBox)
			{
				LockBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			if (LockSprite)
			{
				LockSprite->SetVisibility(false);
				LockSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			if (FlipbookComp)
			{
				// Make the flipbook visibile and play it once
				FlipbookComp->SetVisibility(true);
				FlipbookComp->SetLooping(false);
				FlipbookComp->PlayFromStart();

				FlipbookComp->OnFinishedPlaying.AddDynamic(this, &ATokenUnlockActor::OnDoorFlipbookFinished);

			}
		}
	}

	bCollected = false;
	CollectingCharacter = nullptr;
}

void ATokenUnlockActor::OnDoorFlipbookFinished()
{
	// after the flipbook is finished playing, hide the flipbook component
	for (AActor* LockedActor : LockedActors)
	{
		UPaperFlipbookComponent* FlipbookComp = LockedActor->GetComponentByClass<UPaperFlipbookComponent>();
		if (FlipbookComp)
		{
			FlipbookComp->SetVisibility(false);
		}
	}
}