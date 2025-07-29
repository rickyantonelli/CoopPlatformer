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

	// TODO: Rename these to be appropriate to box/sprite rather than meshes
	TriggerMesh = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerMesh"));
	TriggerMesh->SetupAttachment(RootComp);
	TriggerMesh->SetIsReplicated(true); 

	Mesh = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);

	Activated = false;
	OffsetAmount = 5.0f;
}

void APressurePlate::BeginPlay()
{
	Super::BeginPlay();
	TriggerMesh->OnComponentBeginOverlap.AddDynamic(this, &APressurePlate::OnBoxCollision);
	TriggerMesh->OnComponentEndOverlap.AddDynamic(this, &APressurePlate::OnBoxCollisionEnd);
}

void APressurePlate::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APressurePlate::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (OtherActor->ActorHasTag("Player") && HasAuthority() && PressurePlatedActor)
	{
		UE_LOG(LogTemp, Log, TEXT("PP stepped on"));
		TArray<AActor*> OverlappingActors;
		TriggerMesh->GetOverlappingActors(OverlappingActors);
		if (OverlappingActors.Num() > 1) return;

		MulticastStepOn();
	}
}

void APressurePlate::OnBoxCollisionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->ActorHasTag("Player") && HasAuthority() && PressurePlatedActor)
	{
		UE_LOG(LogTemp, Log, TEXT("PP stepped off"));
		// Need to get overlappingactors instead of just checking OtherActor because we could have both players on the plate
		TArray<AActor*> OverlappingActors;
		TriggerMesh->GetOverlappingActors(OverlappingActors);
		if (OverlappingActors.Num() > 0) return;

		MulticastStepOn();
	}
}

void APressurePlate::MulticastStepOn_Implementation()
{
	UBoxComponent* ActorBox = PressurePlatedActor->GetComponentByClass<UBoxComponent>();
	UPaperSpriteComponent* ActorSprite = PressurePlatedActor->GetComponentByClass<UPaperSpriteComponent>();
	if (ActorBox && ActorSprite)
	{
		if (ActorSprite->IsVisible())
		{
			ActorSprite->SetVisibility(false);
			ActorBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			FVector CurrentLocation = Mesh->GetRelativeLocation();
			FVector NewLocation = CurrentLocation + FVector(0.0f, 0.0f, -OffsetAmount);
			Mesh->SetRelativeLocation(NewLocation);
		}
		else
		{
			ActorSprite->SetVisibility(true);
			ActorBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			FVector CurrentLocation = Mesh->GetRelativeLocation();
			FVector NewLocation = CurrentLocation + FVector(0.0f, 0.0f, OffsetAmount);
			Mesh->SetRelativeLocation(NewLocation);
		}
	}
}


