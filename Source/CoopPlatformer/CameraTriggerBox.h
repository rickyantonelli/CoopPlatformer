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
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** When players leave the collision area */
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere)
	float SpringArmOffset = 0.0f;

	UPROPERTY(EditAnywhere)
	float XOffset = 0.0f;

	UPROPERTY(EditAnywhere)
	float ZOffset = 0.0f;
};
