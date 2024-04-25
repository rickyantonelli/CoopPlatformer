// Copyright Ricky Antonelli


#include "PressurePlate.h"

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

	TriggerMesh->SetVisibility(false);
}

void APressurePlate::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (PressurePlatedActor == nullptr) return;
	UStaticMeshComponent* DoorMesh = PressurePlatedActor->GetComponentByClass<UStaticMeshComponent>();
	if (DoorMesh == nullptr) return;
	DoorMesh->SetVisibility(false);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Activated = true;
}

void APressurePlate::OnBoxCollisionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (PressurePlatedActor == nullptr) return;

	// TODO: need to check if there are any other overlapping actors
	TArray<AActor*> OverlappingActors;
	TriggerMesh->GetOverlappingActors(OverlappingActors);
	if (OverlappingActors.Num() > 0) return;

	UStaticMeshComponent* DoorMesh = PressurePlatedActor->GetComponentByClass<UStaticMeshComponent>();
	if (DoorMesh == nullptr) return;
	DoorMesh->SetVisibility(true);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}


