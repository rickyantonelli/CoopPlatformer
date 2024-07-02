// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "MyPaperCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/InputComponent.h"

#include "BallActor.generated.h"

/** BallActor is the class for the object that players pass back and forth */
UCLASS()
class COOPPLATFORMER_API ABallActor : public AActor
{
	GENERATED_BODY()

public:
	/** Default constructor for ABallActor - Sets components, replication, and initializes variables*/
	ABallActor();

protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

public:
	/** Override for Tick*/
	virtual void Tick(float DeltaTime) override;

	/** Required for replicated variables */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** The root component of the ball actor */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	USceneComponent* RootComp;

	/** The capsule component for the ball*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UCapsuleComponent* Sphere;

	/** The ball's static mesh*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	/** Movement speed for the ball - to be updated in design */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BallMovementSpeed;

	/** Cooldown for passing the ball - which stops players from resetting jump quickly and flying */
	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	float PassCooldownDuration;

	/** Whether the ball passing is on cooldown */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool NoPassCooldown;

	/** If the ball is moving - required for the player controller */
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool IsMoving;

	/** Whether the ball is currently attached to a player */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsAttached, Category = "Debug")
	bool IsAttached;

	/** Whether the ball is ready to be passed*/
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool CanPass;

	/** Begins the pass cooldown */
	UFUNCTION(BlueprintCallable)
	void BeginPassCooldown();

	/** When server changes attachment */
	UFUNCTION()
	void OnRep_IsAttached();
};
