// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeyActor.generated.h"

/** KeyActor is the class that is activated by interaction with the BallActor */
UCLASS()
class COOPPLATFORMER_API AKeyActor : public AActor
{
	GENERATED_BODY()
	
public:
	/** Default constructor for AKeyActor - Sets components, replication, and initializes variables*/
	AKeyActor();

	/** Required for replicated variables - required for passing between players */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

public:
	/** Override for Tick*/
	virtual void Tick(float DeltaTime) override;

	/** The actor that the key will unlock on overlap with the ball */
	UPROPERTY(Replicated, EditAnywhere, Category = "Customizable")
	TObjectPtr<AActor> LockedActor;

	/** The root component of the key actor */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	/** The key's static mesh*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** When the player collides, disables or enables the PressurePlated Actor */
	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** If the key is locked or not, when unlocked stop checking for overlap */
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Debug")
	bool Locked;
};
