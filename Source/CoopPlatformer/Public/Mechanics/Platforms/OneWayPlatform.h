// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "OneWayPlatform.generated.h"

/** The OneWayPlatform class - an obstacle that players may pass from underneath but not from above */
UCLASS()
class COOPPLATFORMER_API AOneWayPlatform : public AActor
{
	GENERATED_BODY()

public:
	/** Default constructor for AOneWayPlatform - Sets components, replication, and initializes variables*/
	AOneWayPlatform();

protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

public:
	/** Override for Tick*/
	virtual void Tick(float DeltaTime) override;

	/** The root component of the one way platform */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	/** The platforms's static mesh*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> Mesh;

	/** The box component for handling collision from below*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> OverlapBox;

	/** When player collides with the box, turns off the player's ECollisionResponse */
	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** When player ends collision with the box, reverts the player's ECollisionResponse */
	UFUNCTION()
	void OnBoxCollisionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};