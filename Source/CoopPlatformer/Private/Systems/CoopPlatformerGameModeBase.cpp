// Copyright Ricky Antonelli


#include "Systems/CoopPlatformerGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Systems/MyPlayerState.h"
#include "Systems/CoopGameInstance.h"

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

	// print the timestamp when a player logs in
	FTimespan TimeSinceStart = FDateTime::Now() - FDateTime(1970, 1, 1);
	UE_LOG(LogTemp, Log, TEXT("Player Logged In at: %f seconds since epoch"), TimeSinceStart.GetTotalSeconds());

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

void ACoopPlatformerGameModeBase::TeleportPlayersToCheckpoints(int32 CheckpointID)
{
	TArray<AActor*> FoundCheckpoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACheckpoint::StaticClass(), FoundCheckpoints);
	if (FoundCheckpoints.IsEmpty()) return;

	ACheckpoint* TargetCheckpoint = nullptr;
	for (AActor* Actor : FoundCheckpoints)
	{
		ACheckpoint* CP = Cast<ACheckpoint>(Actor);
		if (CP && CP->CheckpointID == CheckpointID)
		{
			TargetCheckpoint = CP;
			break;
		}
	}

	if (!TargetCheckpoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportPlayersToCheckpoints: No checkpoint found with ID %d"), CheckpointID);
		return;
	}

	const FVector TargetLoc = TargetCheckpoint->GetActorLocation();
	for (APlayerController* PC : ActiveControllers)
	{
		if (!PC) continue;
		if (AMyPaperCharacter* Pawn = Cast<AMyPaperCharacter>(PC->GetPawn()))
		{
			Pawn->SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);
			Pawn->SpawnLocation = TargetLoc;
		}
	}
}

void ACoopPlatformerGameModeBase::CheckAllPlayersLoaded()
{
	// Wait until both players are connected before checking
	if (ActiveControllers.Num() < 2) return;

	for (APlayerController* PC : ActiveControllers)
	{
		if (!PC) continue;

		AMyPlayerState* PS = PC->GetPlayerState<AMyPlayerState>();
		if (!PS || !PS->IsLoaded)
		{
			return;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("All players loaded! Count: %d"), ActiveControllers.Num());

	// Teleport to checkpoint if one was requested for this travel
	if (UCoopGameInstance* GI = GetGameInstance<UCoopGameInstance>())
	{
		if (GI->PendingCheckpointID > 1)
		{
			TeleportPlayersToCheckpoints(GI->PendingCheckpointID);
		}
		GI->PendingCheckpointID = -1; // always clear regardless

		if (GI->bPendingAttachBall)
		{
			GiveBallToHost();
		}
		GI->bPendingAttachBall = false; // always clear regardless
	}

	for (APlayerController* PC : ActiveControllers)
	{
		if (!PC) continue;
		if (AMyPaperCharacter* Pawn = Cast<AMyPaperCharacter>(PC->GetPawn()))
		{
		Pawn->ClientDismissLoadingScreen();
		}
	}
}

void ACoopPlatformerGameModeBase::GiveBallToHost()
{
	AMyGameStateBase* MyGameState = GetGameState<AMyGameStateBase>();
	if (!MyGameState) return;

	ABallActor* Ball = MyGameState->BallActor;
	if (!Ball) return;

	// Find the host player (bFirstPlayer == true)
	AMyPaperCharacter* HostPlayer = nullptr;
	for (AMyPaperCharacter* Player : MyGameState->ActivePlayers)
	{
		if (Player && Player->bFirstPlayer)
		{
			HostPlayer = Player;
			break;
		}
	}

	if (!HostPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("GiveBallToHost: Could not find host player (bFirstPlayer)."));
		return;
	}

	// Clear any previous holder state
	AMyPaperCharacter* PreviousHolder = MyGameState->GetHolder();
	if (PreviousHolder && PreviousHolder != HostPlayer)
	{
		PreviousHolder->IsHolding = false;
	}

	// Mirror the state set by Controller2D::ServerApplyBallCaught
	Ball->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	MyGameState->SetBallHolder(HostPlayer);
	HostPlayer->IsHolding = true;
	HostPlayer->RemoveBallArrivingClientRPCFunction();
	Ball->CanPass = true;
	Ball->IsAttached = true;
	Ball->IsMoving = false;
	Ball->AttachToComponent(HostPlayer->BallSocket, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	UE_LOG(LogTemp, Log, TEXT("GiveBallToHost: Ball given to %s"), *HostPlayer->GetName());
}