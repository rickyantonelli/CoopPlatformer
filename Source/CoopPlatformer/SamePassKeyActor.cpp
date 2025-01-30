// Copyright Ricky Antonelli


#include "SamePassKeyActor.h"
#include "Net/UnrealNetwork.h"

ASamePassKeyActor::ASamePassKeyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);
}

void ASamePassKeyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASamePassKeyActor, OverlappedMeshes);
}

void ASamePassKeyActor::Tick(float DeltaSeconds)
{
	if (OverlappedMeshes.Num() == KeyMeshes.Num())
	{
		if (HasAuthority()) MulticastTriggerUnlock();
	}

	Super::Tick(DeltaSeconds);
}

void ASamePassKeyActor::BeginPlay()
{
	Super::BeginPlay();

	// Mesh->OnComponentBeginOverlap.AddDynamic(this, &ASamePassKeyActor::OnBoxCollision);
	for (UActorComponent* Component : GetComponents())
	{
		UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Component);
		if (MeshComponent)
		{
			KeyMeshes.Add(MeshComponent);
			MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ASamePassKeyActor::OnBoxCollision);
		}
	}

	AController2D* MyController = Cast<AController2D>(GetWorld()->GetFirstPlayerController());
	if (MyController)
	{
		MyController->OnCaughtActivated.AddDynamic(this, &ASamePassKeyActor::OnBallCaught);
	}
}

void ASamePassKeyActor::MulticastTriggerUnlock_Implementation()
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

void ASamePassKeyActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Ball") && Locked && LockedActors.Num() > 0 && !OverlappedMeshes.Contains(OverlappedComponent) && HasAuthority())
	{
		OverlappedMeshes.Add(OverlappedComponent);
	}
}

void ASamePassKeyActor::OnBallCaught()
{
	// Only care to do this if we still need to unlock this
	if (Locked)
	{
		OverlappedMeshes.Empty();
	}
}
