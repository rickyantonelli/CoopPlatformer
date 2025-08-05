// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Character/MyPaperCharacter.h"
#include "Checkpoint.generated.h"

/** 
* Checkpoint is the class for checkpoints that players can overlap on to trigger
* Requires both players to overlap with it to activate, and can only be activated once
*/
UCLASS()
class COOPPLATFORMER_API ACheckpoint : public AActor
{
	GENERATED_BODY()
	
public:
	/** Default constructor for ACheckpoint - Sets components, replication, and initializes variables*/
	ACheckpoint();

protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

public:	
	/** Override for Tick*/
	virtual void Tick(float DeltaTime) override;

	/** 
	* Adds a player to the array for tracking checkpoint activation - triggered by the player controller 
	* @param PlayerActor: The player character actor to be added to the checkpoint
	*/
	UFUNCTION()
	void AddPlayer(AMyPaperCharacter* PlayerActor);

	/** The root component of the checkpoint */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	/** The trigger mesh for the checkpoint */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> TriggerMesh;

	/** Array of characters that tracks whether a player has overlapped the checkpoint */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	TArray<TObjectPtr<AMyPaperCharacter>> CheckpointedPlayers;
	
	/** Allows the checkpoint to be collected, only allow a checkpoint to be collected once */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool CanBeCollected;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastCheckpointUnlocked();
};
