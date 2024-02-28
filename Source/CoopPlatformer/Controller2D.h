// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "MyPaperCharacter.h"
#include "BallActor.h"


#include "Controller2D.generated.h"

/**
 *
 */
UCLASS()
class COOPPLATFORMER_API AController2D : public APlayerController
{
	GENERATED_BODY()

protected:

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

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void BallPickupServerRPCFunction(AMyPaperCharacter* PlayerActor);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void BallThrownMulticastFunction(float DeltaTime);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void BallThrownServerRPCFunction(float DeltaTime);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void PassMulticastFunction();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void PassServerRPCFunction();

	UFUNCTION(BlueprintCallable)
	void AttachBallToPlayer();// AMyPaperCharacter* PlayerActor);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void GatherPlayersMulticastFunction(const TArray<AMyPaperCharacter*>& UpdatedActivePlayers);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void GatherPlayersServerRPCFunction(const TArray<AMyPaperCharacter*>& UpdatedActivePlayers);
};