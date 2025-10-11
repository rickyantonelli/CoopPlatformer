// Copyright Ricky Antonelli


#include "Mechanics/Keys/TimerKeyActor.h"
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
	if (!Locked) return;

	if (OverlappedMeshes.Num() == KeyMeshes.Num())
	{
		if (HasAuthority()) MulticastTriggerUnlock();
	}

	Super::Tick(DeltaSeconds);
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
		UPaperSpriteComponent* SpriteComponent;
		SpriteComponent = Cast<UPaperSpriteComponent>(OverlappedComponent->GetChildComponent(0));
		if (SpriteComponent) MulticastYellowKey(SpriteComponent);
	}
}

void ATimerKeyActor::MulticastYellowKey_Implementation(UPaperSpriteComponent* SpriteComp)
{
	SpriteComp->SetSprite(YellowKey);
}

void ATimerKeyActor::MulticastTimerExpired_Implementation()
{
	if (Locked) OverlappedMeshes.Empty();
	for (UPaperSpriteComponent* SpriteComp : SpriteComps)
	{
		if (SpriteComp)
		{
			SpriteComp->SetSprite(RedKey);
		}
	}
}

void ATimerKeyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATimerKeyActor, OverlappedMeshes);
}