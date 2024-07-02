// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

/**
 * The player state class for the players in Nova
 */
UCLASS()
class COOPPLATFORMER_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	/** Default constructor for AMyPlayerState - initializes variables*/
	AMyPlayerState();

	/** Whether the player has loaded into the game or not */
	UPROPERTY(Replicated)
	bool IsLoaded;
};
