// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CoopPlatformerGameModeBase.generated.h"

/**
 * The base game mode for Nova
 */
UCLASS()
class COOPPLATFORMER_API ACoopPlatformerGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	/**
	* Overrides the player start, to ensure that both players never spawn on the same player start point
	* @param Player: The player controller
	*/
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/**
	* Overrides the PostLogin
	* @param NewPlayer: The player controller
	*/
	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	
};
