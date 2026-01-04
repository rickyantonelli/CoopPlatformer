// Copyright Ricky Antonelli


#include "Mechanics/Keys/KeyActor.h"
#include "PaperSpriteComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Controller/Controller2D.h"
#include "Net/UnrealNetwork.h"

AKeyActor::AKeyActor()
{
	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Locked = true;

	// We want most things to be non-resettable - aka they stay unlocked
	bCanReset = false;
}

void AKeyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKeyActor, LockedActors);
}

void AKeyActor::BeginPlay()
{
	Super::BeginPlay();

	GetComponents<UPaperSpriteComponent>(SpriteComps);

	if (bCanReset)
	{
		// TODO: Dont use GetFirstPlayerController as its unreliable
		AController2D* MyController = Cast<AController2D>(GetWorld()->GetFirstPlayerController());
		if (MyController)
		{
			MyController->OnResetActivated.AddDynamic(this, &AKeyActor::OnResetActivated);
		}
	}

	UPaperFlipbookComponent* KeyFlipbookComp = GetComponentByClass<UPaperFlipbookComponent>();
	if (KeyFlipbookComp)
	{
		KeyFlipbookComp->OnFinishedPlaying.AddDynamic(this, &AKeyActor::OnUnlockFlipbookFinished);
	}
}

void AKeyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AKeyActor::MulticastRedKey_Implementation()
{
	for (UPaperSpriteComponent* SpriteComp : SpriteComps)
	{
		if (SpriteComp)
		{
			SpriteComp->SetSprite(RedKey);
		}
		UPaperFlipbookComponent* KeyFlipbookComp = GetComponentByClass<UPaperFlipbookComponent>();
		if (KeyFlipbookComp)
		{
			KeyFlipbookComp->SetVisibility(false);
		}
	}
}

void AKeyActor::OnResetActivated()
{
	// when the reset is activated, turn all the LockedActors back to locked, and set Locked bool back
	// this needs to be a multicast, so here all we do is fire that multicast from the server
	if (HasAuthority())
	{
		MulticastTriggerReset();
	}
}

void AKeyActor::OnDoorFlipbookFinished()
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

void AKeyActor::OnUnlockFlipbookFinished()
{
	for (UPaperSpriteComponent* SpriteComp : SpriteComps)
	{
		if (SpriteComp)
		{
			SpriteComp->SetSprite(GreenKey);
			SpriteComp->SetVisibility(true);
			
		}
	}
}

void AKeyActor::MulticastTriggerUnlock_Implementation()
{
	Locked = false;
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

				FlipbookComp->OnFinishedPlaying.AddDynamic(this, &AKeyActor::OnDoorFlipbookFinished);

			}
		}
	}

	// Set the keys to green
	for (UPaperSpriteComponent* SpriteComp : SpriteComps)
	{
		
		if (SpriteComp)
		{
			UPaperFlipbookComponent* KeyFlipbookComp = GetComponentByClass<UPaperFlipbookComponent>();
			if (UnlockFlipbook && KeyFlipbookComp)
			{
				SpriteComp->SetVisibility(false);
				KeyFlipbookComp->SetVisibility(true);
				KeyFlipbookComp->SetLooping(false);
				KeyFlipbookComp->SetFlipbook(UnlockFlipbook);
				KeyFlipbookComp->PlayFromStart();

			}
			else
			{
				SpriteComp->SetSprite(GreenKey);
			}
		}
	}

	// Play the completed sound
	if (CompletedSound)
	{
		UGameplayStatics::PlaySound2D(this, CompletedSound);
	}
}

void AKeyActor::MulticastTriggerReset_Implementation()
{
	Locked = true;
	for (AActor* LockedActor : LockedActors)
	{
		UStaticMeshComponent* LockMesh = LockedActor->GetComponentByClass<UStaticMeshComponent>();
		if (LockMesh)
		{
			LockMesh->SetVisibility(true);
			LockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		else
		{
			UBoxComponent* LockBox = LockedActor->GetComponentByClass<UBoxComponent>();
			UPaperSpriteComponent* LockSprite = LockedActor->GetComponentByClass<UPaperSpriteComponent>();
			if (LockBox)
			{
				LockBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}
			if (LockSprite)
			{
				LockSprite->SetVisibility(true);
				if (!LockBox)
				{
					LockSprite->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				}
			}
		}
	}
	// Set the keys back to red
	for (UPaperSpriteComponent* SpriteComp : SpriteComps)
	{
		if (SpriteComp)
		{
			SpriteComp->SetSprite(RedKey);
		}
	}
}

