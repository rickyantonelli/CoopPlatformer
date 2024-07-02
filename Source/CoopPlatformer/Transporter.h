// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/ArrowComponent.h"
#include "Transporter.generated.h"

/** The transporter class is a component which can be applied to actors and turn them into a moving actor */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPPLATFORMER_API UTransporter : public UActorComponent
{
	GENERATED_BODY()

public:
	UTransporter();

protected:
	/** Default constructor for ABallActor */
	virtual void BeginPlay() override;

public:	
	/** Override for TickComponent*/
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** The point that the actor should start from */
	UPROPERTY(EditAnywhere)
	FVector StartPoint;

	/** The point that the actor should move towards */
	UPROPERTY(EditAnywhere)
	FVector EndPoint;
	
	/** How quickly the actor will move towards its EndPoint */
	UPROPERTY(EditAnywhere, Category = "Customizable")
	float MoveSpeed;

	/**
	* Sets the points for the transporter to travel back and forth towards
	* @param Point1: The first (starting) point
	* @param Point2: The second (end) point
	*/
	UFUNCTION(BlueprintCallable)
	void SetPoints(FVector Point1, FVector Point2);
};
