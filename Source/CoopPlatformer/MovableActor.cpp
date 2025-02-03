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

	// move from point to point every frame
	if (HasAuthority()) // authority because we only want the server to do this
	{
		FVector CurrentLocation = GetActorLocation();
		FVector TargetLocation = EndPoint;
		if (!CurrentLocation.Equals(TargetLocation))
		{
			FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);
			SetActorLocation(NewLocation);
		}
		else
		{
			// if the current location is at the TargetLocation, flip the start and end point so that we go the other way
			FVector Temp = StartPoint;
			StartPoint = EndPoint;
			EndPoint = Temp;
		}
	}
	else
	{
		FVector CurrentLocation = GetActorLocation();
		FVector TargetLocation = EndPoint;
		if (!CurrentLocation.Equals(TargetLocation))
		{
			FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);
			SetActorLocation(NewLocation);
		}
	}

}

void AMovableActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMovableActor, StartPoint);
	DOREPLIFETIME(AMovableActor, EndPoint);
}
