// Copyright Ricky Antonelli


#include "TimerKeyActor.h"
#include "Net/UnrealNetwork.h"

ATimerKeyActor::ATimerKeyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	InTimer = false;
	UnlockTimer = 3.0f;
}

void ATimerKeyActor::BeginPlay()
{
	Super::BeginPlay();

	for (UActorComponent* Component : GetComponents())
	{
		UBoxComponent* MeshComponent = Cast<UBoxComponent>(Component);
		if (MeshComponent)
		{
			KeyMeshes.Add(MeshComponent);
			MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ATimerKeyActor::OnBoxCollision);
		}
	}
}

void ATimerKeyActor::Tick(float DeltaSeconds)
{
	if (OverlappedMeshes.Num() == KeyMeshes.Num())
	{
		if (HasAuthority()) MulticastTriggerUnlock();
	}

	Super::Tick(DeltaSeconds);
}

void ATimerKeyActor::MulticastTriggerUnlock_Implementation()
{
	Locked = false;
	for (AActor* LockedActor : LockedActors)
	{
		// set this to false so that we dont check overlaps anymore since it's already been unlocked
		UStaticMeshComponent* LockMesh = LockedActor->GetComponentByClass<UStaticMeshComponent>();
		if (LockMesh)
		{
			LockMesh->SetVisibility(false);
			LockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ATimerKeyActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Ball") && Locked && LockedActors.Num() > 0 && !OverlappedMeshes.Contains(OverlappedComponent) && HasAuthority())
	{ 
		if (!InTimer)
		{
			// if the timer isn't in action, then start it and mark our bool as true
			FTimerHandle TimerHandler;
			GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() { if (HasAuthority()) MulticastTimerExpired();  }, UnlockTimer, false);
		}

		OverlappedMeshes.Add(OverlappedComponent);
	}
}

void ATimerKeyActor::MulticastTimerExpired_Implementation()
{
	if (Locked) OverlappedMeshes.Empty();
}

void ATimerKeyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATimerKeyActor, OverlappedMeshes);
}