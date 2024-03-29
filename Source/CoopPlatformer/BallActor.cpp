#include "BallActor.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

ABallActor::ABallActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true; // make sure the actor has replication enabled
	SetReplicateMovement(true); // replicating movement is obviously very important

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Sphere = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Sphere->SetupAttachment(RootComp);
	Sphere->SetIsReplicated(true);
	Sphere->SetCollisionProfileName(FName("OverlapAllDynamic"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);
	Mesh->SetCollisionProfileName(FName("OverlapAllDynamic")); // for now lets set this to no collision

	BallMovementSpeed = 100.0f; // just set some base number
	IsMoving = false;
	IsHeld = false;
	CanPass = false;
	IsAttached = false;
	NoPassCooldown = true;

	PassCooldownDuration = 0.5f;
}

void ABallActor::BeginPlay()
{
	Super::BeginPlay();
}


void ABallActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABallActor::BeginPassCooldown()
{
	NoPassCooldown = false;
	FTimerHandle TimerHandler;
	GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {NoPassCooldown = true; }, PassCooldownDuration, false);
}


