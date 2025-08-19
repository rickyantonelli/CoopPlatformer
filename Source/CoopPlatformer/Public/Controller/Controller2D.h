// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Character/MyPaperCharacter.h"
#include "Character/BallActor.h"
#include "Mechanics/Spawn/Checkpoint.h"
#include "Systems/CoopPlatformerGameModeBase.h"
#include "Systems/MyGameStateBase.h"
#include "Controller2D.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBallCaughtActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerResetActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPassActivated);

class USoundBase;

// Define the enum class 
UENUM(BlueprintType)
enum class EHoldingState : uint8
{
	HoldingPlayer = 0 UMETA(DisplayName = "Holding Player"),
	NonHoldingPlayer = 1 UMETA(DisplayName = "Non-Holding Player")
};

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
	/** Default constructor for AController2D - Applies User Settings on construction*/
	AController2D();

	/** Override for Tick*/
	virtual void Tick(float DeltaSeconds) override;

	/** Required for replicated variables - required for passing between players */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** The player that this controller is responsible for - required for camera shifting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AMyPaperCharacter> MyPlayer;

	/** The other player that is not the player that this controller is responsible for - required for camera shifting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AMyPaperCharacter> OtherPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr< ACoopPlatformerGameModeBase> MyGameModeCoop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AMyGameStateBase> MyGameStateCoop;

	/** The ball actor that the players pass back and forth */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TObjectPtr<ABallActor> BallActor;

	/** Delegate for the ball being passed - which is picked up by the player controlller and called on the server */
	UPROPERTY(BlueprintAssignable)
	FBallCaughtActivated OnCaughtActivated;

	UPROPERTY(BlueprintAssignable)
	FPlayerResetActivated OnResetActivated;

	UPROPERTY(BlueprintAssignable)
	FPlayerResetActivated OnPassActivated;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<USoundBase> PassSound;

	/** 
	* Receives delegate for when the player passes the ball 
	* @param PassingPlayer: The player that passed the ball
	*/
	UFUNCTION()
	void OnPassActorActivated();

	/** Server RPC for passing */
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void PassServerRPCFunction();

	/** Overlap begin event, handles checkpoints and player death */
	UFUNCTION()
	void OnOverlapBegin(AActor* PlayerActor, AActor* OtherActor);

	/** Shifts the player's view target to the other player */
	UFUNCTION(BlueprintCallable)
	void ShiftViewTarget();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnCaughtActivated();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastKillBothPlayers();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayPassSound();

	/** Reverts the player's view target back to itself */
	UFUNCTION(BlueprintCallable)
	void RevertViewTarget();

	/** Gathers all actors into variables */
	void GatherActorsHandler();

	/** Handles passing of the ball between players */
	void BallPassingHandler(float DeltaSeconds);

	/** Enforces that we aren't gathering players over again once we have gathered both */
	bool PlayersSet = false;

	void ServerApplyBallCaught();

	UFUNCTION(BlueprintCallable)
	void ValidatePass(AMyPaperCharacter* NewPlayer);

	void ReturnBallToThrower();
};