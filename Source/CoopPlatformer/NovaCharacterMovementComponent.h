// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NovaCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API UNovaCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UNovaCharacterMovementComponent();

	virtual void PhysFalling(float deltaTime, int32 Iterations) override;
	
	UPROPERTY(EditAnywhere)
	float MaxFallSpeed;
};
