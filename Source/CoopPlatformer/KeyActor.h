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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** Override for Tick*/
	virtual void Tick(float DeltaTime) override;

	/** Array of active players - to avoid having to constantly get all actors of class and casting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<AActor*> LockedActors;

	/** The root component of the key actor */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastTriggerUnlock();

	/** If the key is locked or not, when unlocked stop checking for overlap */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool Locked;
};
