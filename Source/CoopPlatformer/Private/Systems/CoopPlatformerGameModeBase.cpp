// Copyright Epic Games, Inc. All Rights Reserved.


#include "Systems/CoopPlatformerGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"


void ACoopPlatformerGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Store the ball actor here
	TArray<AActor*> BallActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "Ball", BallActors);
	AMyGameStateBase* MyGameState = GetGameState<AMyGameStateBase>();
	if (!BallActors.IsEmpty() && HasAuthority() && MyGameState)
	{
		MyGameState->BallActor = Cast<ABallActor>(BallActors[0]);
	}
}

void ACoopPlatformerGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!ActiveControllers.Contains(NewPlayer))
	{
		ActiveControllers.Add(NewPlayer);
	}

	AMyGameStateBase* MyGameState = GetGameState<AMyGameStateBase>();

	if (!PlayersFull)
	{
		UE_LOG(LogTemp, Log, TEXT("Player Reconnect"));
		ACharacter* PlayerCharacter = Cast<ACharacter>(NewPlayer->GetPawn());
		if (PlayerCharacter)
		{
			AMyPaperCharacter* PaperPlayerCharacter = Cast<AMyPaperCharacter>(PlayerCharacter);
			if (PaperPlayerCharacter && MyGameState)
			{
				UE_LOG(LogTemp, Log, TEXT("Player added: %s"), *PaperPlayerCharacter->GetName());


				if (!MyGameState->ActivePlayers.Contains(PaperPlayerCharacter))
				{
					MyGameState->ActivePlayers.Add(PaperPlayerCharacter);
				}


				if (ActiveControllers.Num() == 2) PlayersFull = true;
				else
				{
					PaperPlayerCharacter->bFirstPlayer = true;
				}

				OnPlayersChangedActivated.Broadcast(PaperPlayerCharacter);
			}
		}
	}
}

void ACoopPlatformerGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	UE_LOG(LogTemp, Log, TEXT("Player Disconnect"));
	APlayerController* ExitingController = Cast<APlayerController>(Exiting);

	if (ExitingController)
	{
		PlayersFull = false;
		int32 index = ActiveControllers.Find(ExitingController);
		ActiveControllers.Remove(ExitingController);
		AMyGameStateBase* MyGameState = GetGameState<AMyGameStateBase>();
		if (MyGameState)
		{
			MyGameState->ActivePlayers.RemoveAt(index);
		}
	}
}

void ACoopPlatformerGameModeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACoopPlatformerGameModeBase, ActiveControllers);
	DOREPLIFETIME(ACoopPlatformerGameModeBase, BallActor);
	DOREPLIFETIME(ACoopPlatformerGameModeBase, ActivePlayers);
	DOREPLIFETIME(ACoopPlatformerGameModeBase, PlayersFull);

}

