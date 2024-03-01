


#include "Transporter.h"

UTransporter::UTransporter()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	MoveSpeed = 100.0f;

	StartPoint = FVector::Zero();
	EndPoint = FVector::Zero();
}

void UTransporter::SetPoints(FVector Point1, FVector Point2)
{
	if (Point1.Equals(Point2)) return; // if the 2 points are the same

	StartPoint = Point1;
	EndPoint = Point2;
}

void UTransporter::BeginPlay()
{
	Super::BeginPlay();
	
}

void UTransporter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	AActor* MyOwner = GetOwner(); // returns the owner as an actor pointer
	if (MyOwner && MyOwner->HasAuthority()) // authority because we only want the server to do this
	{
		FVector CurrentLocation = MyOwner->GetActorLocation();
		FVector TargetLocation = EndPoint;
		if (!CurrentLocation.Equals(TargetLocation))
		{
			FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);
			MyOwner->SetActorLocation(NewLocation);
		}
		else
		{
			FVector Temp = StartPoint;
			StartPoint = EndPoint;
			EndPoint = Temp;
		}
	}

}

