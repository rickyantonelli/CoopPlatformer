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

		if (HasAuthority())
		{
			TPActorsOnCD.Add(OtherActor);

			MulticastLaunchPlayer(Player, MoveComp, OverlappedComponent);

			FTimerHandle TimerHandler;
			GetWorld()->GetTimerManager().SetTimer(TimerHandler, [this, OtherActor]() {TPActorsOnCD.Remove(OtherActor); }, TeleportCooldown, false);

		}
	}
}

void ADirectionPortal::MulticastLaunchPlayer_Implementation(AMyPaperCharacter* Player, UCharacterMovementComponent* MoveComp, UPrimitiveComponent* OverlappedComponent)
{
	FVector IncomingVelo = MoveComp->Velocity;

	// for the ball or player, change the actor location to the other teleporter
	UBoxComponent* TargetComp = (OverlappedComponent == TPMesh1) ? TPMesh2 : TPMesh1;
	Player->TeleportTo(TargetComp->GetComponentLocation(), Player->GetActorRotation());

	FVector OutVelo = TargetComp->GetForwardVector() * IncomingVelo.Size();

	Player->LaunchCharacter(OutVelo * LaunchAmp, true, true);

	if (Player->SpringArm)
	{
		Player->SpringArm->CameraLagSpeed += CameraLagOffset;
		FTimerHandle LagTimer;
		GetWorld()->GetTimerManager().SetTimer(LagTimer, [Player, this]() {Player->SpringArm->CameraLagSpeed -= CameraLagOffset; }, CameraLagTime, false);
	}
	MoveComp->FallingLateralFriction = 0;

	GetWorld()->GetTimerManager().ClearTimer(LateralFrictionHandle);
	GetWorld()->GetTimerManager().SetTimer(LateralFrictionHandle, [MoveComp, Player]() {MoveComp->FallingLateralFriction = Player->InitialFriction; }, LateralFrictionTimer, false);
}

void ADirectionPortal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADirectionPortal, TPActorsOnCD);
}
