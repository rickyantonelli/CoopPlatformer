// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "MovableActor.h"
#include "CyclingPlatform.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API ACyclingPlatform : public AMovableActor
{
	GENERATED_BODY()
	
public:
	/** Override for Tick*/
	virtual void Tick(float DeltaTime) override;
};
