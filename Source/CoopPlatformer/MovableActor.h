// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Transporter.h"
#include "MovableActor.generated.h"

/** MovableActor is the class for movable platforms */
UCLASS()
class COOPPLATFORMER_API AMovableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	/** Default constructor for AMovableActor - Sets components, replication, and initializes variables*/
	AMovableActor();

protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

public:
	/** Override for Tick*/
	virtual void Tick(float DeltaTime) override;

	/** The root component of the movable actor */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	/** The first point that the actor moves to */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UArrowComponent> Point1;

	/** The second point that the actor moves to */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UArrowComponent> Point2;

	/** The movable actor's static mesh */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Component that transports the actor back and forth */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UTransporter> Transporter;
};
