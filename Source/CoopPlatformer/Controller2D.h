// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPaperCharacter.h"
#include "BallActor.h"
#include "Checkpoint.h"


#include "Controller2D.generated.h"

/**
 * The player controller class, which handles a variety of responsibilities:
 * Gathering actors
 * Picking up of the ball
 * Passing of the ball
 * Overlap for death and checkpoints
 * Camera swapping for players
 */
UCLASS()
class COOPPLATFORMER_API AController2D : public APlayerController
{
	GENERATED_BODY()

protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

public:
	/** Override for Tick*/
	virtual void Tick(float DeltaSeconds) override;

	/** Required for replicated variables - required for passing between players */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** The player that is holding the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	AMyPaperCharacter* HoldingPlayer;

	/** The player that is not holding the ball - required for passing between players */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	AMyPaperCharacter* NonHoldingPlayer;

	/** The player that this controller is responsible for - required for camera shifting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AMyPaperCharacter* MyPlayer;

	/** The other player that is not the player that this controller is responsible for - required for camera shifting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AMyPaperCharacter* OtherPlayer;

	/** Array of active players - to avoid having to constantly get all actors of class and casting */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TArray<AMyPaperCharacter*> ActivePlayers;

	/** The ball actor that the players pass back and forth */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	ABallActor* BallActor;

	/** 
	* Receives delegate for when the player passes the ball 
	* @param PassingPlayer: The player that passed the ball
	*/
	UFUNCTION()
	void OnPassActorActivated(AMyPaperCharacter* PassingPlayer);

	/** Server RPC for passing */
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void PassServerRPCFunction();

	/** Overlap begin event, handles checkpoints and player death */
	UFUNCTION()
	void OnOverlapBegin(AActor* PlayerActor, AActor* OtherActor);

	/** Shifts the player's view target to the other player */
	UFUNCTION(BlueprintCallable)
	void ShiftViewTarget();

	/** Reverts the player's view target back to itself */
	UFUNCTION(BlueprintCallable)
	void RevertViewTarget();

	/** Gathers all actors into variables */
	void GatherActorsHandler();

	/** Handles picking up of the ball, when it has yet to be picked up before */
	void BallPickupHandler();

	/** Handles passing of the ball between players */
	void BallPassingHandler(float DeltaSeconds);

	/** Enforces that we aren't gathering players over again once we have gathered both */
	bool PlayersSet;
};