// Copyright Ricky Antonelli

#include "Mechanics/Triggers/BallSpeedTrigger.h"
#include "Net/UnrealNetwork.h"

void ABallSpeedTrigger::BeginPlay()
{
	OnActorBeginOverlap.AddDynamic(this, &ABallSpeedTrigger::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &ABallSpeedTrigger::OnOverlapEnd);
}

void ABallSpeedTrigger::OnOverlapBegin(AActor* TriggerBoxActor, AActor* OtherActor)
{
	if (OtherActor->ActorHasTag("Ball") && SpeedOffset != 0.0f)
	{
		// Cast to ball, then change speed
		// Replicated variable so it should just be a matter of changing it?
		ABallActor* BallActor = Cast<ABallActor>(OtherActor);
		if (BallActor)
		{
			BallActor->BallMovementSpeed += SpeedOffset;
		}
	}
}

void ABallSpeedTrigger::OnOverlapEnd(AActor* TriggerBoxActor, AActor* OtherActor)
{
	if (OtherActor->ActorHasTag("Ball") && SpeedOffset != 0.0f)
	{
		// Cast to ball, then change speed
		// Replicated variable so it should just be a matter of changing it?
		ABallActor* BallActor = Cast<ABallActor>(OtherActor);
		if (BallActor)
		{
			BallActor->BallMovementSpeed -= SpeedOffset;
		}
	}
}
