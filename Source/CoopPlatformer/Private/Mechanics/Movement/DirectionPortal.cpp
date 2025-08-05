// Copyright Ricky Antonelli

#include "Mechanics/Movement/DirectionPortal.h"
#include "Character/MyPaperCharacter.h"

ADirectionPortal::ADirectionPortal()
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
	LateralFrictionTimer = 0.5f;
	LaunchAmp = 1.0f;
}

void ADirectionPortal::BeginPlay()
{
	Super::BeginPlay();

	TPMesh1->OnComponentBeginOverlap.AddDynamic(this, &ADirectionPortal::OnBoxBeginOverlap);
	TPMesh2->OnComponentBeginOverlap.AddDynamic(this, &ADirectionPortal::OnBoxBeginOverlap);
	
}

void ADirectionPortal::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (CanTeleport && OtherActor->ActorHasTag("Player"))
	{

		CanTeleport = false;

		AMyPaperCharacter* Player = Cast<AMyPaperCharacter>(OtherActor);
		if (Player && Player->SpringArm)
		{
			Player->SpringArm->CameraLagSpeed += CameraLagOffset;
			FTimerHandle LagTimer;
			GetWorld()->GetTimerManager().SetTimer(LagTimer, [Player, this]() {Player->SpringArm->CameraLagSpeed -= CameraLagOffset; }, CameraLagTime, false);

			UCharacterMovementComponent* MoveComp = Player->FindComponentByClass<UCharacterMovementComponent>();
			if (MoveComp)
			{
				FVector IncomingVelo = MoveComp->Velocity;
				UE_LOG(LogTemp, Warning, TEXT("IncomingVelo Velocity: %s"), *IncomingVelo.ToString());

				float LateralFriction = MoveComp->FallingLateralFriction;

				MoveComp->FallingLateralFriction = 0;
				// for the ball or player, change the actor location to the other teleporter
				UBoxComponent* TargetComp = (OverlappedComponent == TPMesh1) ? TPMesh2 : TPMesh1;
				Player->SetActorLocation(TargetComp->GetComponentLocation());
				float Speed = IncomingVelo.Size();
				UE_LOG(LogTemp, Warning, TEXT("TargetComp->GetForwardVector(): %s"), *TargetComp->GetForwardVector().ToString());
				FVector OutVelo = TargetComp->GetForwardVector() * Speed;
				UE_LOG(LogTemp, Warning, TEXT("Outgoing Velocity: %s"), *OutVelo.ToString());

				Player->LaunchCharacter(OutVelo * LaunchAmp, true, true);

				FTimerHandle LateralFrictionHandle;
				GetWorld()->GetTimerManager().SetTimer(LateralFrictionHandle, [MoveComp, LateralFriction]() {MoveComp->FallingLateralFriction = LateralFriction; }, LateralFrictionTimer, false);

				FTimerHandle TimerHandler;
				GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {CanTeleport = true; }, TeleportCooldown, false);

			}

		}
	}
}

