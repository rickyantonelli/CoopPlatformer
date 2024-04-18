// Copyright Epic Games, Inc. All Rights Reserved.


#include "CoopPlatformerGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

AActor* ACoopPlatformerGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	// ensures that player's dont start on the same start point
	TArray<AActor*> StartActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), StartActors);
	for (AActor* Actor : StartActors)
	{
		if (!Actor->ActorHasTag("Taken"))
		{
			APlayerStart* StartActor = Cast<APlayerStart>(Actor);
			StartActor->Tags.Add("Taken");
			return StartActor;
		}
	}

	return Super::ChoosePlayerStart(Player);
}