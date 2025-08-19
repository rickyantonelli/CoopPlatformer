// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Character/BallActor.h"
#include "Character/MyPaperCharacter.h"
#include "Systems/MyGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameModeBase.h"
#include "CoopPlatformerGameModeBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayersChangedActivated, AMyPaperCharacter*, NewPlayer);

/**
 * The base game mode for Nova
 */
UCLASS()
class COOPPLATFORMER_API ACoopPlatformerGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	/**
	* Overrides the PostLogin
	* @param NewPlayer: The player controller
	*/
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	/** Required for replicated variables - required for passing between players */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;



public:

	/** Array of active players - to avoid having to constantly get all actors of class and casting */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TArray<APlayerController*> ActiveControllers;

	/** Array of active players - to avoid having to constantly get all actors of class and casting */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TArray<AMyPaperCharacter*> ActivePlayers;

	/** The ball actor that the players pass back and forth */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	ABallActor* BallActor;

	/** Whether the ball is ready to be passed*/
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool PlayersFull = false;

	UPROPERTY(BlueprintAssignable)
	FPlayersChangedActivated OnPlayersChangedActivated;

protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;
};