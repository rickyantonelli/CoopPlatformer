// Copyright Ricky Antonelli


#include "Mechanics/Movement/SplitPlayersTeleporter.h"
#include "Character/MyPaperCharacter.h"

// Sets default values
ASplitPlayersTeleporter::ASplitPlayersTeleporter()
{

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Teleporter = CreateDefaultSubobject<UBoxComponent>(TEXT("Teleporter"));
	Teleporter->SetupAttachment(RootComp);
	Teleporter->SetIsReplicated(true);

	P1Teleporter = CreateDefaultSubobject<UBoxComponent>(TEXT("P1Teleporter"));
	P1Teleporter->SetupAttachment(RootComp);
	P1Teleporter->SetIsReplicated(true);

	P2Teleporter = CreateDefaultSubobject<UBoxComponent>(TEXT("P2Teleporter"));
	P2Teleporter->SetupAttachment(RootComp);
	P2Teleporter->SetIsReplicated(true);

	TeleportCooldown = 0.5f;
	CameraLagOffset = -5.0f;
	CameraLagTime = 1.0f;

}

// Called when the game starts or when spawned
void ASplitPlayersTeleporter::BeginPlay()
{
	Super::BeginPlay();

	Teleporter->OnComponentBeginOverlap.AddDynamic(this, &ASplitPlayersTeleporter::OnTeleportDistribute);
	P1Teleporter->OnComponentBeginOverlap.AddDynamic(this, &ASplitPlayersTeleporter::OnTeleportReturn);
	P2Teleporter->OnComponentBeginOverlap.AddDynamic(this, &ASplitPlayersTeleporter::OnTeleportReturn);
	
}

void ASplitPlayersTeleporter::OnTeleportDistribute(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// no ball teleporting, only players
	if (OtherActor->ActorHasTag("Player") && !TPActorsOnCD.Contains(OtherActor))
	{
		TPActorsOnCD.Add(OtherActor);
		ApplyCameraLag(OtherActor);

		// for the ball or player, change the actor location to the other teleporter
		// FVector TargetLocation = (OverlappedComponent == TPMesh1) ? TPMesh2->GetComponentLocation() : TPMesh1->GetComponentLocation();
		FVector TargetLocation = FVector::ZeroVector;
		if (bP1Taken)
		{
			TargetLocation = P2Teleporter->GetComponentLocation();
		}
		else
		{
			TargetLocation = P1Teleporter->GetComponentLocation();
			bP1Taken = true;
		}
		OtherActor->SetActorLocation(TargetLocation);
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [this, OtherActor]() {TPActorsOnCD.Remove(OtherActor); }, TeleportCooldown, false);
	}
}

void ASplitPlayersTeleporter::OnTeleportReturn(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Player") && !TPActorsOnCD.Contains(OtherActor))
	{
		TPActorsOnCD.Add(OtherActor);
		ApplyCameraLag(OtherActor);
		
		FVector TargetLocation = Teleporter->GetComponentLocation();
		
		// if overlappedcomponent is P1Teleporter, make bP1Taken false
		if (OverlappedComponent->GetName() == P1Teleporter->GetName())
		{
			bP1Taken = false;
		}

		OtherActor->SetActorLocation(TargetLocation);
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [this, OtherActor]() {TPActorsOnCD.Remove(OtherActor); }, TeleportCooldown, false);
	}
}

void ASplitPlayersTeleporter::ApplyCameraLag(AActor* PlayerActor)
{
	AMyPaperCharacter* Player = Cast<AMyPaperCharacter>(PlayerActor);
	if (Player && Player->SpringArm)
	{
		Player->SpringArm->CameraLagSpeed += CameraLagOffset;
		FTimerHandle LagTimer;
		GetWorld()->GetTimerManager().SetTimer(LagTimer, [Player, this]() {Player->SpringArm->CameraLagSpeed -= CameraLagOffset; }, CameraLagTime, false);
	}
}
