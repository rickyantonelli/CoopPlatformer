// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPaperCharacter.h"
#include "BallActor.h"
#include "Checkpoint.h"


#include "Controller2D.generated.h"

/**
 *
 */
UCLASS()
class COOPPLATFORMER_API AController2D : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere)
	AMyPaperCharacter* HoldingPlayer;

	UPROPERTY(VisibleAnywhere)
	AMyPaperCharacter* NonHoldingPlayer;

	UPROPERTY(VisibleAnywhere)
	TArray<AMyPaperCharacter*> ActivePlayers;

	UPROPERTY(VisibleAnywhere)
	ABallActor* BallActor;

	UFUNCTION()
	void OnPassActorActivated();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void BallPickupMulticastFunction(AMyPaperCharacter* PlayerActor);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void PassMulticastFunction();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void PassServerRPCFunction();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void GatherPlayersMulticastFunction(const TArray<AMyPaperCharacter*>& UpdatedActivePlayers);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void PlayerDeathMulticastFunction(AMyPaperCharacter* PlayerActor);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void CheckpointActivatedMulticastFunction(AMyPaperCharacter* PlayerActor, ACheckpoint* Checkpoint);

	UFUNCTION()
	void OnOverlapBegin(AActor* PlayerActor, AActor* OtherActor);

	void GatherActorsHandler();

	void BallPickupHandler();

	void BallPassingHandler(float DeltaSeconds);
};