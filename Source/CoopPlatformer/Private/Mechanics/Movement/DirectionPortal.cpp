// Copyright Ricky Antonelli

#include "Mechanics/Movement/DirectionPortal.h"
#include "Character/MyPaperCharacter.h"
#include "Net/UnrealNetwork.h"

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

	TeleportCooldown = 0.5f;
	CameraLagOffset = -5.0f;
	CameraLagTime = 1.0f;
	LateralFrictionTimer = 0.5f;
	LaunchAmp = 1.0f;
	MovementDisableAmount = 0.1f;

	bBallAllowed = false;
}

void ADirectionPortal::BeginPlay()
{
	Super::BeginPlay();

	TPMesh1->OnComponentBeginOverlap.AddDynamic(this, &ADirectionPortal::OnBoxBeginOverlap);
	TPMesh2->OnComponentBeginOverlap.AddDynamic(this, &ADirectionPortal::OnBoxBeginOverlap);
}

void ADirectionPortal::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Player") && !TPActorsOnCD.Contains(OtherActor))
	{
		AMyPaperCharacter* Player = Cast<AMyPaperCharacter>(OtherActor);
		if (!Player) return;
		UCharacterMovementComponent* MoveComp = Player->FindComponentByClass<UCharacterMovementComponent>();
		if (!MoveComp) return;

		Player->DisableJump(MovementDisableAmount);

		if (HasAuthority())
		{
			TPActorsOnCD.Add(OtherActor);

			MulticastLaunchPlayer(Player, MoveComp, OverlappedComponent);

			FTimerHandle TimerHandler;
			GetWorld()->GetTimerManager().SetTimer(TimerHandler, [this, OtherActor]() {TPActorsOnCD.Remove(OtherActor); }, TeleportCooldown, false);

		}
	}

	if (bBallAllowed && OtherActor->ActorHasTag("Ball") && !TPActorsOnCD.Contains(OtherActor))
	{
		// Check if the Ball is attached to a Player
		AActor* ParentActor = OtherActor->GetAttachParentActor();
		if (ParentActor && ParentActor->ActorHasTag("Player"))
		{
			// The Ball is attached to a Player, do not teleport
			return;
		}

		if (HasAuthority())
		{
			TPActorsOnCD.Add(OtherActor);

			FVector TargetLocation = (OverlappedComponent == TPMesh1) ? TPMesh2->GetComponentLocation() : TPMesh1->GetComponentLocation();
			OtherActor->TeleportTo(TargetLocation, OtherActor->GetActorRotation());

			FTimerHandle TimerHandler;
			GetWorld()->GetTimerManager().SetTimer(TimerHandler, [this, OtherActor]() {TPActorsOnCD.Remove(OtherActor); }, TeleportCooldown, false);

		}

	}
}

void ADirectionPortal::MulticastLaunchPlayer_Implementation(AMyPaperCharacter* Player, UCharacterMovementComponent* MoveComp, UPrimitiveComponent* OverlappedComponent)
{
	FVector IncomingVelo = MoveComp->Velocity;

	UBoxComponent* TargetComp = (OverlappedComponent == TPMesh1) ? TPMesh2 : TPMesh1;
	Player->TeleportTo(TargetComp->GetComponentLocation(), Player->GetActorRotation());

	FVector OutVelo = TargetComp->GetForwardVector() * IncomingVelo.Size();
	Player->LaunchCharacter(OutVelo * LaunchAmp, true, true);

	if (Player->SpringArm)
	{
		Player->SpringArm->CameraLagSpeed += CameraLagOffset;
		FTimerHandle LagTimer;
		GetWorld()->GetTimerManager().SetTimer(LagTimer, [Player, this]()
			{
				if (IsValid(Player) && Player->SpringArm) // Check validity
				{
					Player->SpringArm->CameraLagSpeed -= CameraLagOffset;
				}
			}, CameraLagTime, false);
	}

	// Use the player's own MulticastApplyFriction function instead
	if (Player)
	{
		Player->MulticastApplyFriction(0, LateralFrictionTimer);
	}
}

void ADirectionPortal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADirectionPortal, TPActorsOnCD);
}
