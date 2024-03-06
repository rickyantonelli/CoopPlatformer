// Fill out your copyright notice in the Description page of Project Settings.


#include "Checkpoint.h"

// Sets default values
ACheckpoint::ACheckpoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

	HasBeenGathered = false;
	Tags.Add("Checkpoint");
	
}

// Called when the game starts or when spawned
void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();
	//OnActorBeginOverlap.AddDynamic(this, &ACheckpoint::OnOverlapBegin);
}

// Called every frame
void ACheckpoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACheckpoint::AddPlayer(AMyPaperCharacter* PlayerActor)
{
	if (CheckpointedPlayers.Find(PlayerActor) == -1)
	{
		CheckpointedPlayers.Add(PlayerActor);
	}
	if (CheckpointedPlayers.Num() == 2)
	{
		for (AMyPaperCharacter* CheckpointedActor : CheckpointedPlayers)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Checkpoint!");
			CheckpointedActor->SpawnLocation = GetActorLocation();
		}
	}
}

