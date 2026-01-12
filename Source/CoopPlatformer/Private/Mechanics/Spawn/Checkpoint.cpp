// Copyright Ricky Antonelli

#include "Mechanics/Spawn/Checkpoint.h"
#include "PaperFlipbook.h"

// Sets default values
ACheckpoint::ACheckpoint()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true; // enabling replication for the actor itself, the components we will do down below
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp")); 
	SetRootComponent(RootComp); 

	CheckpointSprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("CheckpointSprite"));
	CheckpointSprite->SetupAttachment(RootComp);
	CheckpointSprite->SetIsReplicated(true);
	CheckpointSprite->SetCollisionProfileName(FName("OverlapAllDynamic"));

	CanBeCollected = true;
	Tags.Add("Checkpoint");
	
}

void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();


	UnlockFlipbook = FindComponentByClass<UPaperFlipbookComponent>();

	if (UnlockFlipbook)
	{
		UnlockFlipbook->OnFinishedPlaying.AddDynamic(this, &ACheckpoint::OnUnlockFlipbookFinished);
		// Flipbook should be hidden and set replicated in the editor
	}
}

void ACheckpoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACheckpoint::AddPlayer(AMyPaperCharacter* PlayerActor)
{
	// Adds player to checkpoint array
	if (CanBeCollected)
	{
		if (CheckpointedPlayers.Find(PlayerActor) == -1)
		{
			// add player if it's not already in our array
			CheckpointedPlayers.Add(PlayerActor);
		}
		if (CheckpointedPlayers.Num() == 2)
		{
			// once we get to two players in the array then we are good to move the checkpoint for both players
			for (AMyPaperCharacter* CheckpointedActor : CheckpointedPlayers)
			{
				if (!CheckpointedActor) continue;
				CheckpointedActor->SpawnLocation = GetActorLocation();
				CanBeCollected = false;
				if (HasAuthority())
				{
					CheckpointedActor->ActiveCheckpoint = CheckpointID;
					// visually turn off the checkpoint
					MulticastCheckpointUnlocked();
				}
			}
		}
	}
}

void ACheckpoint::MulticastCheckpointUnlocked_Implementation()
{
	if (CheckpointSprite)
	{
		CheckpointSprite->SetVisibility(false);
	}

	if (UnlockFlipbook)
	{
		UnlockFlipbook->SetVisibility(true);
		UnlockFlipbook->SetLooping(false);
		UnlockFlipbook->PlayFromStart();
	}
}

void ACheckpoint::OnUnlockFlipbookFinished()
{
	if (UnlockFlipbook)
	{
		UnlockFlipbook->SetVisibility(false);
	}
}

