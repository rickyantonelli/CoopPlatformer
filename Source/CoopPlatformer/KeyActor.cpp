// Fill out your copyright notice in the Description page of Project Settings.


#include "KeyActor.h"
#include "BallActor.h"

// Sets default values
AKeyActor::AKeyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

// Called when the game starts or when spawned
void AKeyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AKeyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Locked) // only check if the key is locked
	{
		TArray<AActor*> OverlapActors;
		GetOverlappingActors(OverlapActors, ABallActor::StaticClass());
		if (!OverlapActors.IsEmpty())
		{
			for (AActor* AA : OverlapActors)
			{
				if (AA->ActorHasTag("Ball"))
				{
					// we've found the ball actor lets do the logic
					Locked = false;
					if (LockActor)
					{
						//LockActor->GetDefaultSubobjectByName("Mesh");
						UStaticMeshComponent* LockMesh = LockActor->GetComponentByClass<UStaticMeshComponent>();
						if (LockMesh)
						{
							LockMesh->SetVisibility(false);
							LockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						}
					}
					break;
				}
			}
		}
	}

}

