// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CoopPlatformerGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API ACoopPlatformerGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	
};
