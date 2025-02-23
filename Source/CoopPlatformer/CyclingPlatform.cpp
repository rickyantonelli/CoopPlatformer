// Copyright Ricky Antonelli


#include "CyclingPlatform.h"

void ACyclingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// move from point to point every frame
	if (!GetActorLocation().Equals(EndPoint))
	{
		FVector NewLocation = FMath::VInterpConstantTo(GetActorLocation(), EndPoint, DeltaTime, MoveSpeed);
		SetActorLocation(NewLocation);
	}
	else
	{
		// if the current location is at the TargetLocation, just reset this back to the start point
		SetActorLocation(StartPoint);
	}
}
