// Copyright Ricky Antonelli


#include "PassKeyActor.h"

APassKeyActor::APassKeyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	Mesh = CreateDefaultSubobject<UBoxComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);
}

void APassKeyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APassKeyActor::BeginPlay()
{
	Super::BeginPlay();

	Mesh->OnComponentBeginOverlap.AddDynamic(this, &APassKeyActor::OnBoxCollision);
}

void APassKeyActor::MulticastTriggerUnlock_Implementation()
{
	for (AActor* LockedActor : LockedActors)
	{
		Locked = false; // set this to false so that we dont check overlaps anymore since it's already been unlocked
		UStaticMeshComponent* LockMesh = LockedActor->GetComponentByClass<UStaticMeshComponent>();
		if (LockMesh)
		{
			LockMesh->SetVisibility(false);
			LockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void APassKeyActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Ball") && Locked && LockedActors.Num() > 0)
	{
		if (HasAuthority()) MulticastTriggerUnlock();
	}
}
