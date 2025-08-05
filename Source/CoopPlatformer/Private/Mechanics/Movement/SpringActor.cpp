// Copyright Ricky Antonelli


#include "Mechanics/Movement/SpringActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASpringActor::ASpringActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	BoxMesh = CreateDefaultSubobject<UBoxComponent>(TEXT("Mesh"));
	BoxMesh->SetupAttachment(RootComp);
	BoxMesh->SetIsReplicated(true);

	LaunchPower = 1000;

}

// Called when the game starts or when spawned
void ASpringActor::BeginPlay()
{
	Super::BeginPlay();
	
	BoxMesh->OnComponentBeginOverlap.AddDynamic(this, &ASpringActor::OnBoxCollision);
}

// Called every frame
void ASpringActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpringActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Player"))
	{
		AMyPaperCharacter* MyCharacter = Cast<AMyPaperCharacter>(OtherActor);
		if (MyCharacter)
		{
			FVector LaunchDirection = FVector(0, 0, 1);
			MyCharacter->LaunchCharacter(LaunchDirection * LaunchPower, false, true);
		}
	}
}

