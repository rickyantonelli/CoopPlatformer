// Copyright Ricky Antonelli


#include "ResetKeyActor.h"

AResetKeyActor::AResetKeyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	Mesh = CreateDefaultSubobject<UBoxComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);
}

void AResetKeyActor::BeginPlay()
{
	Super::BeginPlay();

	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AResetKeyActor::OnBoxCollision);
}

void AResetKeyActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Ball") && Locked && LockedActors.Num() > 0)
	{
		if (HasAuthority())
		{
			MulticastTriggerUnlock();

			FTimerHandle TimerHandler;
			GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() { if (HasAuthority()) MulticastTriggerReset();  }, ResetTimer, false);
		}
	}
}