// Copyright Ricky Antonelli

#include "MovableActor.h"
#include "Net/UnrealNetwork.h"

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

	Mesh = CreateDefaultSubobject<UBoxComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);

	MoveSpeed = 100.0f; // just a default value, this is customizable
}

void AMovableActor::BeginPlay()
{
	Super::BeginPlay();

	// set points for the transporter to keep track of, and the transporter component will do the moving of the object
	StartPoint = GetActorLocation() + Point1->GetRelativeLocation();
	EndPoint = GetActorLocation() + Point2->GetRelativeLocation();
}

void AMovableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMovableActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMovableActor, StartPoint);
	DOREPLIFETIME(AMovableActor, EndPoint);
}
