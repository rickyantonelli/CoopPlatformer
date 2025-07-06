// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "BallActor.h"
#include "Engine/TriggerBox.h"
#include "BallSpeedTrigger.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API ABallSpeedTrigger : public ATriggerBox
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	void OnOverlapBegin(AActor* TriggerBoxActor, AActor* OtherActor);

	/** When players leave the collision area */
	UFUNCTION()
	void OnOverlapEnd(AActor* TriggerBoxActor, AActor* OtherActor);

	UPROPERTY(EditAnywhere)
	float SpeedOffset = 0.0f;

};
