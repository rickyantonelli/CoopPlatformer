// Copyright Ricky Antonelli


#include "Mechanics/Keys/SamePassKeyActor.h"
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
		if (HasAuthority() && Locked) MulticastTriggerUnlock();
	}

	Super::Tick(DeltaSeconds);
}

void ASamePassKeyActor::BeginPlay()
{
	Super::BeginPlay();

	// Mesh->OnComponentBeginOverlap.AddDynamic(this, &ASamePassKeyActor::OnBoxCollision);
	for (UActorComponent* Component : GetComponents())
	{
		UBoxComponent* MeshComponent = Cast<UBoxComponent>(Component);
		if (MeshComponent)
		{
			KeyMeshes.Add(MeshComponent);
			MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ASamePassKeyActor::OnBoxCollision);
		}
	}
}

void ASamePassKeyActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() && OtherActor->ActorHasTag("Ball") && Locked && LockedActors.Num() > 0 && !OverlappedMeshes.Contains(OverlappedComponent))
	{
		OverlappedMeshes.Add(OverlappedComponent);
		UPaperSpriteComponent* SpriteComponent;
		SpriteComponent = Cast<UPaperSpriteComponent>(OverlappedComponent->GetChildComponent(0));
		if (SpriteComponent) MulticastYellowKey(SpriteComponent);
	}
}

void ASamePassKeyActor::MulticastYellowKey_Implementation(UPaperSpriteComponent* SpriteComp)
{
	SpriteComp->SetSprite(YellowKey);
}

void ASamePassKeyActor::OnBallCaught()
{
	// Only care to do this if we still need to unlock this
	if (Locked)
	{
		OverlappedMeshes.Empty();
		MulticastRedKey();
	}
}
