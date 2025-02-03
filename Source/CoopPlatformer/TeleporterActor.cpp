// Copyright Ricky Antonelli


#include "TeleporterActor.h"

// Sets default values
ATeleporterActor::ATeleporterActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	TPMesh1 = CreateDefaultSubobject<UBoxComponent>(TEXT("TPMesh1"));
	TPMesh1->SetupAttachment(RootComp);
	TPMesh1->SetIsReplicated(true);

	TPMesh2 = CreateDefaultSubobject<UBoxComponent>(TEXT("TPMesh2"));
	TPMesh2->SetupAttachment(RootComp);
	TPMesh2->SetIsReplicated(true);

	CanTeleport = true;
	TeleportCooldown = 0.5f;

}

// Called when the game starts or when spawned
void ATeleporterActor::BeginPlay()
{
	Super::BeginPlay();

	TPMesh1->OnComponentBeginOverlap.AddDynamic(this, &ATeleporterActor::OnBoxBeginOverlap);
	TPMesh2->OnComponentBeginOverlap.AddDynamic(this, &ATeleporterActor::OnBoxBeginOverlap);
}

// Called every frame
void ATeleporterActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATeleporterActor::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (CanTeleport && (OtherActor->ActorHasTag("Ball") || OtherActor->ActorHasTag("Player")))
	{
		if (OtherActor->ActorHasTag("Ball"))
		{
			// Check if the Ball is attached to a Player
			AActor* ParentActor = OtherActor->GetAttachParentActor();
			if (ParentActor && ParentActor->ActorHasTag("Player"))
			{
				// The Ball is attached to a Player, do not teleport
				return;
			}
		}

		CanTeleport = false;
		// for the ball or player, change the actor location to the other teleporter
		FVector TargetLocation = (OverlappedComponent == TPMesh1) ? TPMesh2->GetComponentLocation() : TPMesh1->GetComponentLocation();
		OtherActor->SetActorLocation(TargetLocation);
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {CanTeleport = true; }, TeleportCooldown, false);
	}
}

