// Copyright Ricky Antonelli

#include "Transporter.h"
#include "Net/UnrealNetwork.h"

UTransporter::UTransporter()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	MoveSpeed = 100.0f; // just a default value, this is customizable

	StartPoint = FVector::Zero();
	EndPoint = FVector::Zero();
}

void UTransporter::SetPoints(FVector Point1, FVector Point2)
{
	if (Point1.Equals(Point2)) return; // if the 2 points are the same we dont want to do anything

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

	// move from point to point every frame
	AActor* MyOwner = GetOwner();
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
			// if the current location is at the TargetLocation, flip the start and end point so that we go the other way
			FVector Temp = StartPoint;
			StartPoint = EndPoint;
			EndPoint = Temp;
		}
	}
	else
	{
		FVector CurrentLocation = MyOwner->GetActorLocation();
		FVector TargetLocation = EndPoint;
		if (!CurrentLocation.Equals(TargetLocation))
		{
			FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);
			MyOwner->SetActorLocation(NewLocation);
		}
	}

}

void UTransporter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTransporter, StartPoint);
	DOREPLIFETIME(UTransporter, EndPoint);

}

