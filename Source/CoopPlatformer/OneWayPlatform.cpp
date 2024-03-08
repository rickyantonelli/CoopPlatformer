// Fill out your copyright notice in the Description page of Project Settings.


#include "OneWayPlatform.h"

// Sets default values
AOneWayPlatform::AOneWayPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);
	Mesh->SetCollisionProfileName("OneWayPlatform");

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComp);
	Box->SetIsReplicated(true);
}

// Called when the game starts or when spawned
void AOneWayPlatform::BeginPlay()
{
	Super::BeginPlay();
	Box->OnComponentBeginOverlap.AddDynamic(this, &AOneWayPlatform::OnBoxCollision);
	Box->OnComponentEndOverlap.AddDynamic(this, &AOneWayPlatform::OnBoxCollisionEnd);
	
}

// Called every frame 
void AOneWayPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOneWayPlatform::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Player"))
	{

		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Starting");
		
		OtherComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel5, ECollisionResponse::ECR_Ignore);

	}
}

void AOneWayPlatform::OnBoxCollisionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->ActorHasTag("Player"))
	{

		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Ending");

		OtherComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel5, ECollisionResponse::ECR_Block);

	}

}

