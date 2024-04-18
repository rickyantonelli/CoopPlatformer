// Copyright Ricky Antonelli

#include "Checkpoint.h"

// Sets default values
ACheckpoint::ACheckpoint()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true; // enabling replication for the actor itself, the components we will do down below
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp")); 
	SetRootComponent(RootComp); 

	TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
	TriggerMesh->SetupAttachment(RootComp);
	TriggerMesh->SetIsReplicated(true);
	TriggerMesh->SetCollisionProfileName(FName("OverlapAllDynamic"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp); 
	Mesh->SetIsReplicated(true);
	Mesh->SetCollisionProfileName(FName("OverlapAllDynamic"));

	CanBeCollected = true;
	Tags.Add("Checkpoint");
	
}

void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();
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
				CheckpointedActor->SpawnLocation = GetActorLocation();
				CanBeCollected = false;
			}
		}
	}
}

