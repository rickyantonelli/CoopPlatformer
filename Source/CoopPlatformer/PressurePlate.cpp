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

	DOREPLIFETIME(APressurePlate, Activated);
}

void APressurePlate::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (PressurePlatedActor == nullptr) return;
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

	if (HasAuthority())
	{
		Activated = true;
	}
}

void APressurePlate::OnBoxCollisionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (PressurePlatedActor == nullptr) return;

	TArray<AActor*> OverlappingActors;
	TriggerMesh->GetOverlappingActors(OverlappingActors);
	if (OverlappingActors.Num() > 0) return;

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


