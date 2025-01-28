// Copyright Ricky Antonelli


#include "PassKeyActor.h"

APassKeyActor::APassKeyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);
}

void APassKeyActor::BeginPlay()
{
	Super::BeginPlay();

	Mesh->OnComponentBeginOverlap.AddDynamic(this, &APassKeyActor::OnBoxCollision);
}

void APassKeyActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Ball"))
	{
		if (Locked && LockedActor) // only check if the key is locked
		{
			TArray<AActor*> OverlapActors;
			GetOverlappingActors(OverlapActors, ABallActor::StaticClass());
			if (!OverlapActors.IsEmpty())
			{
				for (AActor* AA : OverlapActors)
				{
					if (AA->ActorHasTag("Ball") && LockedActor)
					{
						// we've found the ball actor lets do the logic
						if (HasAuthority()) Locked = false; // set this to false so that we dont check overlaps anymore since it's already been unlocked
						UStaticMeshComponent* LockMesh = LockedActor->GetComponentByClass<UStaticMeshComponent>();
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
}
