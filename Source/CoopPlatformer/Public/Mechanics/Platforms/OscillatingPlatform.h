// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Mechanics/Platforms/MovableActor.h"
#include "OscillatingPlatform.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API AOscillatingPlatform : public AMovableActor
{
	GENERATED_BODY()
	
public:
	/** Override for Tick*/
	virtual void Tick(float DeltaTime) override;
};
