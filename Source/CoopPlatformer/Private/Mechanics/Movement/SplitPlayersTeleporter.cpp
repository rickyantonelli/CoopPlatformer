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

void ASplitPlayersTeleporter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (P1Player)
	{
		P1Player->OnPlayerDeathStarted.RemoveDynamic(this, &ASplitPlayersTeleporter::HandlePlayerDeath);
	}

	if (P2Player && P2Player != P1Player)
	{
		P2Player->OnPlayerDeathStarted.RemoveDynamic(this, &ASplitPlayersTeleporter::HandlePlayerDeath);
	}

	Super::EndPlay(EndPlayReason);
}

void ASplitPlayersTeleporter::OnTeleportDistribute(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMyPaperCharacter* Player = Cast<AMyPaperCharacter>(OtherActor);
	if (Player && !Player->bDead && !TPActorsOnCD.Contains(Player))
	{
		FVector TargetLocation = FVector::ZeroVector;
		if (!P1Player)
		{
			P1Player = Player;
			bP1Taken = true;
			TargetLocation = P1Teleporter->GetComponentLocation();
		}
		else if (!P2Player)
		{
			P2Player = Player;
			TargetLocation = P2Teleporter->GetComponentLocation();
		}
		else
		{
			return;
		}

		TPActorsOnCD.Add(Player);
		ApplyCameraLag(Player);
		Player->OnPlayerDeathStarted.AddUniqueDynamic(this, &ASplitPlayersTeleporter::HandlePlayerDeath);
		Player->TeleportTo(TargetLocation, Player->GetActorRotation());
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [this, Player]() {TPActorsOnCD.Remove(Player); }, TeleportCooldown, false);
	}
}

void ASplitPlayersTeleporter::OnTeleportReturn(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMyPaperCharacter* Player = Cast<AMyPaperCharacter>(OtherActor);
	if (Player && !Player->bDead && !TPActorsOnCD.Contains(Player))
	{
		TPActorsOnCD.Add(Player);
		ApplyCameraLag(Player);
		
		FVector TargetLocation = Teleporter->GetComponentLocation();
		
		if (P1Player == Player) P1Player = nullptr;
		if (P2Player == Player) P2Player = nullptr;
		bP1Taken = P1Player != nullptr;
		Player->OnPlayerDeathStarted.RemoveDynamic(this, &ASplitPlayersTeleporter::HandlePlayerDeath);

		Player->TeleportTo(TargetLocation, Player->GetActorRotation());
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [this, Player]() {TPActorsOnCD.Remove(Player); }, TeleportCooldown, false);
	}
}

void ASplitPlayersTeleporter::HandlePlayerDeath(AMyPaperCharacter* Player)
{
	if (!Player) return;

	TPActorsOnCD.Remove(Player);

	if (P1Player == Player) P1Player = nullptr;
	if (P2Player == Player) P2Player = nullptr;
	bP1Taken = P1Player != nullptr;

	Player->OnPlayerDeathStarted.RemoveDynamic(this, &ASplitPlayersTeleporter::HandlePlayerDeath);
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
