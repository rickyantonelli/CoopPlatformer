// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Character/MyPaperCharacter.h"
#include "Character/BallActor.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameStateResetActivated);

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	/** Array of active players - to avoid having to constantly get all actors of class and casting */
	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	//TArray<APlayerController*> ActiveControllers;

	/** Array of active players - to avoid having to constantly get all actors of class and casting */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TArray<AMyPaperCharacter*> ActivePlayers;

	/** The player currently holding the ball — single source of truth for ball possession */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	AMyPaperCharacter* BallHolder = nullptr;

	/** The ball actor that the players pass back and forth */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	ABallActor* BallActor;

	/** Required for replicated variables - required for passing between players */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayPassSound(USoundBase* Sound);

	/** Returns the player currently holding the ball (may be null if no one holds it) */
	AMyPaperCharacter* GetHolder() const { return BallHolder; }

	/** Returns the player who is NOT the ball holder. Returns nullptr if fewer than 2 players or no holder set. */
	AMyPaperCharacter* GetReceiver() const
	{
		if (!BallHolder || ActivePlayers.Num() < 2) return nullptr;
		for (AMyPaperCharacter* Player : ActivePlayers)
		{
			if (Player && Player != BallHolder)
			{
				return Player;
			}
		}
		return nullptr;
	}

	void SetBallHolder(AMyPaperCharacter* NewHolder) { BallHolder = NewHolder; }

	/** Broadcast when a full reset occurs — enemies and keys bind to this instead of a specific player controller */
	UPROPERTY(BlueprintAssignable)
	FGameStateResetActivated OnResetActivated;
};
