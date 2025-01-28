// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "BallActor.h"
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

public:
	/** Override for Tick*/
	virtual void Tick(float DeltaTime) override;

	/** The actor that the key will unlock on overlap with the ball */
	UPROPERTY(Replicated, EditAnywhere, Category = "Customizable")
	TObjectPtr<AActor> LockedActor;

	/** The root component of the key actor */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	UFUNCTION()
	void TriggerUnlock();

	/** If the key is locked or not, when unlocked stop checking for overlap */
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Debug")
	bool Locked;
};
