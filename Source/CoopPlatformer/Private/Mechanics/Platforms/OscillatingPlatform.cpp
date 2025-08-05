// Copyright Ricky Antonelli


#include "Mechanics/Platforms/OscillatingPlatform.h"

void AOscillatingPlatform::Tick(float DeltaTime)
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
