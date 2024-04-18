// Copyright Ricky Antonelli

#include "MovableActor.h"

AMovableActor::AMovableActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Point1 = CreateDefaultSubobject<UArrowComponent>(TEXT("Point1"));
	Point1->SetupAttachment(RootComp);
	Point1->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	Point2 = CreateDefaultSubobject<UArrowComponent>(TEXT("Point2"));
	Point2->SetupAttachment(RootComp);
	Point2->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);

	Transporter = CreateDefaultSubobject<UTransporter>(TEXT("Transporter"));

}

void AMovableActor::BeginPlay()
{
	Super::BeginPlay();

	// set points for the transporter to keep track of, and the transporter component will do the moving of the object
	FVector StartPoint = GetActorLocation() + Point1->GetRelativeLocation();
	FVector EndPoint = GetActorLocation() + Point2->GetRelativeLocation();

	Transporter->SetPoints(StartPoint, EndPoint);
}

void AMovableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

