// Copyright Ricky Antonelli

#include "OneWayPlatform.h"

// Sets default values
AOneWayPlatform::AOneWayPlatform()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	// this one way platform has a Mesh and a Box
	// the Mesh is your standard mesh, the Box sits below the mesh to be able to identify collision from below

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);
	Mesh->SetCollisionProfileName("OneWayPlatform");

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComp);
	Box->SetIsReplicated(true);
}

void AOneWayPlatform::BeginPlay()
{
	Super::BeginPlay();
	Box->OnComponentBeginOverlap.AddDynamic(this, &AOneWayPlatform::OnBoxCollision);
	Box->OnComponentEndOverlap.AddDynamic(this, &AOneWayPlatform::OnBoxCollisionEnd);
}

void AOneWayPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOneWayPlatform::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Player"))
	{
		// we ignore collision with the mesh because the player is entering from below
		// ECC_GameTraceChannel5 is the channel that the mesh is tied to
		// we use a channel because there are multiple players - if we just turn off the mesh then it could make the other player fall through
		OtherComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel5, ECollisionResponse::ECR_Ignore);
	}
}

void AOneWayPlatform::OnBoxCollisionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->ActorHasTag("Player"))
	{
		// we turn collision with the mesh back on
		OtherComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel5, ECollisionResponse::ECR_Block);
	}

}

