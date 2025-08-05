// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Character/MyPaperCharacter.h"
#include "Character/BallActor.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	/** Array of active players - to avoid having to constantly get all actors of class and casting */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TArray<AMyPaperCharacter*> ActivePlayers;

	/** The ball actor that the players pass back and forth */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	ABallActor* BallActor;

	/** Required for replicated variables - required for passing between players */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
