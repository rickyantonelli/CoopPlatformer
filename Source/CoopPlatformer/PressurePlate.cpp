// Copyright Ricky Antonelli


#include "PressurePlate.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APressurePlate::APressurePlate()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp")); 
	SetRootComponent(RootComp);

	TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
	TriggerMesh->SetupAttachment(RootComp);
	TriggerMesh->SetIsReplicated(true); 

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);

	Activated = false;

}

void APressurePlate::BeginPlay()
{
	Super::BeginPlay();
	TriggerMesh->OnComponentBeginOverlap.AddDynamic(this, &APressurePlate::OnBoxCollision);
	TriggerMesh->OnComponentEndOverlap.AddDynamic(this, &APressurePlate::OnBoxCollisionEnd);

	if (HasAuthority())
	{
		TriggerMesh->SetVisibility(false);
	}
}

void APressurePlate::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APressurePlate::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (HasAuthority())
	{
		if (PressurePlatedActor == nullptr) return;
		MulticastStepOn();
	}
}

void APressurePlate::OnBoxCollisionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority())
	{
		if (PressurePlatedActor == nullptr) return;

		// Need to get overlappingactors instead of just checking OtherActor because we could have both players on the plate
		TArray<AActor*> OverlappingActors;
		TriggerMesh->GetOverlappingActors(OverlappingActors);
		if (OverlappingActors.Num() > 0) return;

		MulticastStepOff();
	}
}

void APressurePlate::MulticastStepOn_Implementation()
{
	UStaticMeshComponent* DoorMesh = PressurePlatedActor->GetComponentByClass<UStaticMeshComponent>();
	if (DoorMesh == nullptr) return;

	// check for visibility here so that the we have the flexibility
	// to have the pressure plate turn on or off
	if (DoorMesh->IsVisible())
	{
		// disable visibility and collision
		DoorMesh->SetVisibility(false);
		DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		// enable visibility and collision
		DoorMesh->SetVisibility(true);
		DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	Activated = true;
}

void APressurePlate::MulticastStepOff_Implementation()
{
	UStaticMeshComponent* DoorMesh = PressurePlatedActor->GetComponentByClass<UStaticMeshComponent>();
	if (DoorMesh == nullptr) return;

	if (DoorMesh->IsVisible())
	{
		DoorMesh->SetVisibility(false);
		DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		DoorMesh->SetVisibility(true);
		DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}


