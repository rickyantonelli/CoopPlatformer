// Copyright Ricky Antonelli


#include "KeyActor.h"
#include "BallActor.h"
#include "Net/UnrealNetwork.h"

AKeyActor::AKeyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);

	Locked = true;

}

void AKeyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKeyActor, Locked);
	DOREPLIFETIME(AKeyActor, LockedActor);
}

void AKeyActor::BeginPlay()
{
	Super::BeginPlay();

	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AKeyActor::OnBoxCollision);
	// TriggerMesh->OnComponentBeginOverlap.AddDynamic(this, &APressurePlate::OnBoxCollision);
}

void AKeyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AKeyActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

