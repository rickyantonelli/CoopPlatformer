// Copyright Ricky Antonelli

#include "Mechanics/Movement/TeleporterActor.h"
#include "Character/MyPaperCharacter.h"

ATeleporterActor::ATeleporterActor()
{
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
	CameraLagOffset = -5.0f;
	CameraLagTime = 1.0f;

}

void ATeleporterActor::BeginPlay()
{
	Super::BeginPlay();

	TPMesh1->OnComponentBeginOverlap.AddDynamic(this, &ATeleporterActor::OnBoxBeginOverlap);
	TPMesh2->OnComponentBeginOverlap.AddDynamic(this, &ATeleporterActor::OnBoxBeginOverlap);
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

		AMyPaperCharacter* Player = Cast<AMyPaperCharacter>(OtherActor);
		if (Player && Player->SpringArm)
		{
			Player->SpringArm->CameraLagSpeed += CameraLagOffset;
			FTimerHandle LagTimer;
			GetWorld()->GetTimerManager().SetTimer(LagTimer, [Player, this]() {Player->SpringArm->CameraLagSpeed -= CameraLagOffset; }, CameraLagTime, false);
		}

		// for the ball or player, change the actor location to the other teleporter
		FVector TargetLocation = (OverlappedComponent == TPMesh1) ? TPMesh2->GetComponentLocation() : TPMesh1->GetComponentLocation();
		OtherActor->SetActorLocation(TargetLocation);
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {CanTeleport = true; }, TeleportCooldown, false);
	}
}

