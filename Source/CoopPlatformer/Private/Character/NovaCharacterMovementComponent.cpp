// Copyright Ricky Antonelli


#include "Character/NovaCharacterMovementComponent.h"

UNovaCharacterMovementComponent::UNovaCharacterMovementComponent()
{
	MaxFallSpeed = -500;
}

void UNovaCharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	Super::PhysFalling(deltaTime, Iterations);

	// We accelerate too quickly when falling - cap it
	if (Velocity.Z < MaxFallSpeed)
	{
		Velocity.Z = MaxFallSpeed;
	}
}
