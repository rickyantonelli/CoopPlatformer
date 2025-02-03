// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "MyPaperCharacter.h"
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

	/** Required for replicated variables */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UBoxComponent> Mesh;

	UPROPERTY(EditAnywhere, Replicated)
	FVector StartPoint;

	/** The point that the actor should move towards */
	UPROPERTY(EditAnywhere, Replicated)
	FVector EndPoint;

	/** How quickly the actor will move towards its EndPoint */
	UPROPERTY(EditAnywhere, Category = "Customizable")
	float MoveSpeed;
};
