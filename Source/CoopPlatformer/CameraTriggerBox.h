// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "CameraTriggerBox.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API ACameraTriggerBox : public ATriggerBox
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
	float SpringArmOffset = 0.0f;

	UPROPERTY(EditAnywhere)
	float XOffset = 0.0f;

	UPROPERTY(EditAnywhere)
	float ZOffset = 0.0f;
};
