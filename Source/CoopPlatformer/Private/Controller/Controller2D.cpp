// Copyright Ricky Antonelli

#include "Controller/Controller2D.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/GameStateBase.h"
#include "Mechanics/Keys/SamePassKeyActor.h"
#include "Systems/MyPlayerState.h"
#include "Mechanics/Movement/DashToken.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

AController2D::AController2D()
{
}

void AController2D::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!PlayersSet)
	{
		// sets the arrays for the players and the ballactor object - so once these are set we never need to check or call again
		GatherActorsHandler();
	}
	BallPassingHandler(DeltaSeconds);

}


void AController2D::BeginPlay()
{
	Super::BeginPlay();

	AGameModeBase* MyGameMode = GetWorld()->GetAuthGameMode();
	MyGameModeCoop = Cast<ACoopPlatformerGameModeBase>(MyGameMode);

	if (MyGameModeCoop)
	{
		MyGameModeCoop->OnPlayersChangedActivated.AddDynamic(this, &AController2D::ValidatePass);
	}

	// MyGameStateCoop = GetWorld()->GetGameState<AMyPlayerState>();
	MyGameStateCoop = GetWorld()->GetGameState<AMyGameStateBase>();

	// Get the ball actor, which is stored in the game mode
	// The ball just exists in each level, so it should always be there in a level

	// TODO: Make this a check() later, cause we want a crash here if the ball doesn't exist
	if (HasAuthority() && MyGameStateCoop && MyGameStateCoop->BallActor)
	{
		BallActor = MyGameStateCoop->BallActor;
	}

	// Apply the user settings we want as default
	// Eventually this will go away
	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
	if (UserSettings)
	{
		int settingQuality = 2;
		UserSettings->SetViewDistanceQuality(settingQuality);
		UserSettings->SetAntiAliasingQuality(settingQuality);
		UserSettings->SetShadowQuality(settingQuality);
		UserSettings->SetPostProcessingQuality(settingQuality);
		UserSettings->SetTextureQuality(settingQuality);
		UserSettings->SetVisualEffectQuality(settingQuality);
		UserSettings->SetFoliageQuality(settingQuality);
		UserSettings->SetShadingQuality(settingQuality);
		UserSettings->SetGlobalIlluminationQuality(settingQuality);
		UserSettings->SetReflectionQuality(settingQuality);

		UserSettings->ApplySettings(true);

		// Some debugging to ensure the settings were correctly set
		UE_LOG(LogTemp, Log, TEXT("Resolution Quality: %f"), UserSettings->GetResolutionScaleNormalized());
		UE_LOG(LogTemp, Log, TEXT("View Distance Quality: %d"), UserSettings->GetViewDistanceQuality());
		UE_LOG(LogTemp, Log, TEXT("Anti-Aliasing Quality: %d"), UserSettings->GetAntiAliasingQuality());
		UE_LOG(LogTemp, Log, TEXT("Shadow Quality: %d"), UserSettings->GetShadowQuality());
		UE_LOG(LogTemp, Log, TEXT("Post-Process Quality: %d"), UserSettings->GetPostProcessingQuality());
		UE_LOG(LogTemp, Log, TEXT("Texture Quality: %d"), UserSettings->GetTextureQuality());
		UE_LOG(LogTemp, Log, TEXT("Effects Quality: %d"), UserSettings->GetVisualEffectQuality());
		UE_LOG(LogTemp, Log, TEXT("Foliage Quality: %d"), UserSettings->GetFoliageQuality());
		UE_LOG(LogTemp, Log, TEXT("Shading Quality: %d"), UserSettings->GetShadingQuality());
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASamePassKeyActor::StaticClass(), FoundActors);
	for (AActor* Actor : FoundActors)
	{
		ASamePassKeyActor* KeyActor = Cast<ASamePassKeyActor>(Actor);
		if (KeyActor)
		{
			OnCaughtActivated.AddDynamic(KeyActor, &ASamePassKeyActor::OnBallCaught);
			UE_LOG(LogTemp, Log, TEXT("Bound OnCaughtActivated to %s"), *KeyActor->GetName());
		}
	}

	PlayersSet = false;
}

void AController2D::OnPassActorActivated()
{
	// delegate received from the player
	PassServerRPCFunction();
}

void AController2D::OnCountdownPingActivated()
{
	CountdownPingServerRPCFunction();
}

void AController2D::PassServerRPCFunction_Implementation()
{
	if (!BallActor) return;
	if (!BallActor->CanPass) return;
	if (!BallActor->NoPassCooldown) return;
	if (MyGameStateCoop->ActivePlayers.Num() != 2) return;
	//if (!MyGameStateCoop->ActivePlayers[0] || !MyGameStateCoop->ActivePlayers[1]) return;
	if (!PlayersSet) return;
	if (BallActor->GetAttachParentActor() != MyGameStateCoop->ActivePlayers[0]) return;

	BallActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	BallActor->IsAttached = false;
	MyGameStateCoop->ActivePlayers[0]->IsHolding = false;
	BallActor->IsMoving = true;
	BallActor->CanPass = false;
	BallActor->BeginPassCooldown();

	// TODO: do we need this?
	BallActor->ForceNetUpdate();

	MyGameStateCoop->ActivePlayers[1]->BallArrivingClientRPCFunction();
	MulticastPlayPassSound();

	OnPassActivated.Broadcast();
}

void AController2D::BallPassingHandler(float DeltaSeconds)
{
	// When the ball is passed and while it is traveling, this gets called every frame

	if (BallActor && BallActor->IsMoving && PlayersSet)
	{
			
		// TODO: This should be moved to OnOverlapBegin - inefficient to constantly check an array of overlap actors
		TArray<AActor*> OverlapActors;
		MyGameStateCoop->ActivePlayers[1]->GetOverlappingActors(OverlapActors, ABallActor::StaticClass());
		if (!OverlapActors.IsEmpty())
		{
			// The ball has arrived at the NonHoldingPlayer
			ServerApplyBallCaught();
		}
		else
		{
			FVector NewLocation = FMath::VInterpConstantTo(BallActor->GetActorLocation(), MyGameStateCoop->ActivePlayers[1]->GetActorLocation(), DeltaSeconds, BallActor->BallMovementSpeed); // this takes in 2 vectors and moves towards the target location at the given speed - this specifically returns the new location we need to be at
			BallActor->SetActorLocation(NewLocation);
		}
	}
}

void AController2D::ServerApplyBallCaught()
{
	if (HasAuthority())
	{
		// Apply all the needed things when a ball is caught - reset jump, swap array positions, set variables, attachment
		MyGameStateCoop->ActivePlayers[1]->ResetJumpAbility(); // resets the jump for the player if they are in mid-air (core mechanic)
		BallActor->CanPass = true;
		BallActor->IsAttached = true;
		MyGameStateCoop->ActivePlayers.Swap(0,1);
		MyGameStateCoop->ActivePlayers[0]->IsHolding = true;
		MyGameStateCoop->ActivePlayers[0]->RemoveBallArrivingWidget();
		BallActor->AttachToActor(MyGameStateCoop->ActivePlayers[0], FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		BallActor->IsMoving = false;

		// Broadcast that the ball was caught
		MulticastOnCaughtActivated();
	}
}

void AController2D::ValidatePass(AMyPaperCharacter* NewPlayer)
{
	// TODO: we need a delegate that fires from the game mode whenever we need to re-check activeplayers
	// basically do a re-validation, ensure there are 2 players
	// then do some logic to check if one of the 2 players is holding the ball (if so set up the array in the right order)
	// if no one is holding then set things back to the initial check or whatever
	if (!NewPlayer->OnPassActivated.IsAlreadyBound(this, &AController2D::OnPassActorActivated))
	{
		NewPlayer->OnPassActivated.AddDynamic(this, &AController2D::OnPassActorActivated);
	}

	if (!NewPlayer->OnActorBeginOverlap.IsAlreadyBound(this, &AController2D::OnOverlapBegin))
	{
		NewPlayer->OnActorBeginOverlap.AddDynamic(this, &AController2D::OnOverlapBegin);
	}

	if (!NewPlayer->OnCountdownPingActivated.IsAlreadyBound(this, &AController2D::OnCountdownPingActivated))
	{
		NewPlayer->OnCountdownPingActivated.AddDynamic(this, &AController2D::OnCountdownPingActivated);
	}

	MyGameStateCoop->ActivePlayers[1]->SetActorLocation(MyGameStateCoop->ActivePlayers[0]->GetActorLocation());
}

void AController2D::ReturnBallToThrower()
{
	// To be called when the ball is in mid air
	if (HasAuthority() && BallActor->IsMoving)
	{
		MyGameStateCoop->ActivePlayers.Swap(0, 1);
		MyGameStateCoop->ActivePlayers[1]->BallArrivingClientRPCFunction();
	}
}

void AController2D::GatherActorsHandler()
{
	if (MyGameStateCoop && MyGameStateCoop->ActivePlayers.Num() == 2 && !PlayersSet)
	{
		for (AMyPaperCharacter* Actor : MyGameStateCoop->ActivePlayers)
		{
			if (Actor)
			{
				if (!Actor->OnPassActivated.IsAlreadyBound(this, &AController2D::OnPassActorActivated))
				{
					Actor->OnPassActivated.AddDynamic(this, &AController2D::OnPassActorActivated);
				}

				if (!Actor->OnActorBeginOverlap.IsAlreadyBound(this, &AController2D::OnOverlapBegin))
				{
					Actor->OnActorBeginOverlap.AddDynamic(this, &AController2D::OnOverlapBegin);
				}

				if (!Actor->OnCountdownPingActivated.IsAlreadyBound(this, &AController2D::OnCountdownPingActivated))
				{
					Actor->OnCountdownPingActivated.AddDynamic(this, &AController2D::OnCountdownPingActivated);
				}
			}
		}
		PlayersSet = true;
	}
}

void AController2D::CountdownPingServerRPCFunction_Implementation()
{
	if (MyGameStateCoop->ActivePlayers.Num() != 2) return;
	MyGameStateCoop->ActivePlayers[0]->CountdownPingClientRPCFunction();
	MyGameStateCoop->ActivePlayers[1]->CountdownPingClientRPCFunction();
}

void AController2D::OnOverlapBegin(AActor *PlayerActor, AActor* OtherActor)
{
	// ball pickup handling
	if (HasAuthority() && OtherActor->ActorHasTag("Ball") && BallActor && !BallActor->IsAttached && !BallActor->IsMoving && MyGameStateCoop->ActivePlayers.Num() == 2)
	{
		AMyPaperCharacter* OverlappingActor = Cast<AMyPaperCharacter>(PlayerActor);

		if (MyGameStateCoop->ActivePlayers[0] != OverlappingActor)
		{
			MyGameStateCoop->ActivePlayers.Swap(0, 1);
		}

		MyGameStateCoop->ActivePlayers[0]->IsHolding = true;

		BallActor->AttachToActor(MyGameStateCoop->ActivePlayers[0], FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		BallActor->CanPass = true;
		BallActor->IsAttached = true;
	}


	// death handling
	if (OtherActor->ActorHasTag("Death"))
	{
		AMyPaperCharacter* PlayerCharacterActor = Cast<AMyPaperCharacter>(PlayerActor);
		if (PlayerCharacterActor)
		{
			if (PlayerCharacterActor->IsHolding)
			{
				PassServerRPCFunction();
			}
			// send the ball back to the thrower if player dies while ball is on its way
			else if (BallActor && BallActor->IsMoving && PlayerCharacterActor == MyGameStateCoop->ActivePlayers[1])
			{
				ReturnBallToThrower();
			}

			// The player dies - returning them to spawn (or checkpoint) and disabling movement for a short amount of time
			UCharacterMovementComponent* MyCharacterMovement = PlayerCharacterActor->GetCharacterMovement();
			if (MyCharacterMovement)
			{
				// reset the movement to zero so that the momentum doesn't continue when the player respawns
				MyCharacterMovement->Velocity = FVector::ZeroVector;
			}
			PlayerCharacterActor->SetActorLocation(PlayerCharacterActor->SpawnLocation);
			PlayerCharacterActor->OnDeath();
		}
	}

	if (OtherActor->ActorHasTag("FullDeath"))
	{
		// this tag implies a death type that requires a full reset (kills both players)
		if (HasAuthority())
		{
			MulticastKillBothPlayers();
		}
	}

	// checkpoint handling
	if (OtherActor->ActorHasTag("Checkpoint"))
	{
		AMyPaperCharacter* PlayerCharacterActor = Cast<AMyPaperCharacter>(PlayerActor);
		ACheckpoint* CheckpointActor = Cast<ACheckpoint>(OtherActor);
		if (PlayerCharacterActor && CheckpointActor)
		{
			if (CheckpointActor->CheckpointedPlayers.Find(PlayerCharacterActor) == -1) // if the player actor is not in our Tarray for the checkpoint
			{
				CheckpointActor->AddPlayer(PlayerCharacterActor);
			}
		}
	}

	// Dash prototype - holding off for now
	if (OtherActor->ActorHasTag("Dash"))
	{
		AMyPaperCharacter* PlayerCharacterActor = Cast<AMyPaperCharacter>(PlayerActor);
		PlayerCharacterActor->ApplyDashToken();

		ADashToken* DashToken = Cast<ADashToken>(OtherActor);
		DashToken->CollectDash();
	}
}

void AController2D::ShiftViewTarget()
{
	if (!MyPlayer || !OtherPlayer)
	{
		for (AMyPaperCharacter* ActivePlayer : MyGameStateCoop->ActivePlayers)
		{
			APlayerController* ActivePlayerController = ActivePlayer->GetLocalViewingPlayerController();
			if (ActivePlayerController == this)
			{
				MyPlayer = ActivePlayer;

			}
			else
			{
				OtherPlayer = ActivePlayer;
			}
		}
	}
	SetViewTargetWithBlend(OtherPlayer, 0.3f, EViewTargetBlendFunction::VTBlend_Linear, 2.0f);
}

void AController2D::RevertViewTarget()
{
	SetViewTargetWithBlend(MyPlayer, 0.3f, EViewTargetBlendFunction::VTBlend_Linear, 2.0f);
}

void AController2D::MulticastOnCaughtActivated_Implementation()
{
	OnCaughtActivated.Broadcast();
}

void AController2D::MulticastKillBothPlayers_Implementation()
{
	// trigger death for both players
	for (AMyPaperCharacter* ActivePlayer : MyGameStateCoop->ActivePlayers)
	{
		ActivePlayer->SetActorLocation(ActivePlayer->SpawnLocation);
		ActivePlayer->OnDeath();
	}
	OnResetActivated.Broadcast();
}

void AController2D::MulticastPlayPassSound_Implementation()
{
	// Play the completed sound on all clients
	if (PassSound)
	{
		UGameplayStatics::PlaySound2D(this, PassSound);
	}
}

void AController2D::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AController2D, BallActor);
}