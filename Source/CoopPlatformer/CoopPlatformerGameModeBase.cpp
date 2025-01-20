// Copyright Epic Games, Inc. All Rights Reserved.


#include "CoopPlatformerGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"


void ACoopPlatformerGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Store the ball actor here
	TArray<AActor*> BallActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "Ball", BallActors);
	if (!BallActors.IsEmpty())
	{
		BallActor = Cast<ABallActor>(BallActors[0]);
	}
}

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

void ACoopPlatformerGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ActiveControllers.Add(NewPlayer);

}

void ACoopPlatformerGameModeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACoopPlatformerGameModeBase, ActiveControllers);
	DOREPLIFETIME(ACoopPlatformerGameModeBase, BallActor);

}

