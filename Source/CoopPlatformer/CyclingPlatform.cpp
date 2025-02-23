// Copyright Ricky Antonelli


#include "CyclingPlatform.h"

void ACyclingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// move from point to point every frame
	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = EndPoint;
	if (!CurrentLocation.Equals(TargetLocation))
	{
		FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);
		SetActorLocation(NewLocation);
	}
	else
	{
		// if the current location is at the TargetLocation, just reset this back to the start point
		SetActorLocation(StartPoint);
	}
}
