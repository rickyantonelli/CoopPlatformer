#include "BallActor.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

ABallActor::ABallActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true; // make sure the actor has replication enabled
	SetReplicateMovement(true); // replicating movement is obviously very important
	// SetReplicates(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Sphere = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Sphere->SetupAttachment(RootComp);
	Sphere->SetIsReplicated(true);
	Sphere->SetCollisionProfileName(FName("OverlapAllDynamic"));

	BallMovementSpeed = 100.0f; // to be tweaked in the engine
	PassCooldownDuration = 0.5f;

	IsMoving = false;
	CanPass = false;
	IsAttached = false;
	NoPassCooldown = true;
	
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
	// set a slight pass cooldown so that players cant infinitely pass back and forth and fly
	NoPassCooldown = false;
	FTimerHandle TimerHandler;
	GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {NoPassCooldown = true; }, PassCooldownDuration, false);
}

void ABallActor::OnRep_IsAttached()
{
	if (!IsAttached)
	{
		// DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		BeginPassCooldown();
	}
}

void ABallActor::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABallActor, IsMoving);
	DOREPLIFETIME(ABallActor, IsAttached);
	DOREPLIFETIME(ABallActor, CanPass);
	DOREPLIFETIME(ABallActor, BallMovementSpeed);
}



