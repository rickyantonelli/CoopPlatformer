// Copyright Ricky Antonelli


#include "KeyActor.h"
#include "BallActor.h"

AKeyActor::AKeyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);

	Locked = true;

}

void AKeyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AKeyActor::Tick(float DeltaTime)
{
	// TODO: Change this from GetOverlappingActors() to OnOverlapBegin()
	Super::Tick(DeltaTime);
	if (Locked) // only check if the key is locked
	{
		TArray<AActor*> OverlapActors;
		GetOverlappingActors(OverlapActors, ABallActor::StaticClass());
		if (!OverlapActors.IsEmpty())
		{
			for (AActor* AA : OverlapActors)
			{
				if (AA->ActorHasTag("Ball") && LockActor)
				{
					// we've found the ball actor lets do the logic
					Locked = false; // set this to false so that we dont check overlaps anymore since it's already been unlocked
					UStaticMeshComponent* LockMesh = LockActor->GetComponentByClass<UStaticMeshComponent>();
					if (LockMesh)
					{
						LockMesh->SetVisibility(false);
						LockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					}
					// no need to keep looping 
					break;
				}
			}
		}
	}

}

