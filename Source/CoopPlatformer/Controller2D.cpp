// Copyright Ricky Antonelli

#include "Controller2D.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "MyPlayerState.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "CoopPlatformerGameModeBase.h"
#include "DashToken.h"

AController2D::AController2D()
{
}

void AController2D::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GatherActorsHandler();
	BallPickupHandler();
	BallPassingHandler(DeltaSeconds);

	APlayerState* MyPlayerState = GetPlayerState<APlayerState>();
	if (MyPlayerState)
	{
		float Ping = MyPlayerState->ExactPing;
		GEngine->AddOnScreenDebugMessage(-1, 0.01, FColor::Red, FString::Printf(TEXT("Ping is: %f"), Ping));
	}
}

void AController2D::BeginPlay()
{
	Super::BeginPlay();

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

	PlayersSet = false;
}


void AController2D::OnPassActorActivated(AMyPaperCharacter* PassingPlayer)
{
	// delegate received from the player
	PassServerRPCFunction();
}

void AController2D::PassServerRPCFunction_Implementation()
{
	if (BallActor && BallActor->CanPass && BallActor->NoPassCooldown && HoldingPlayer && NonHoldingPlayer && BallActor->GetAttachParentActor() == HoldingPlayer)
	{
		BallActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		BallActor->IsAttached = false;
		HoldingPlayer->IsHolding = false;
		BallActor->IsMoving = true;
		BallActor->CanPass = false;
		BallActor->BeginPassCooldown();

		BallActor->ForceNetUpdate();

		NonHoldingPlayer->BallArrivingClientRPCFunction();
	}
}

void AController2D::BallPickupHandler()
{
	// This only will get called once per level - when the ball is not being held by anyone yet

	// TODO: A lot of checks here and there is likely redundancy
	if (HasAuthority() && BallActor && !BallActor->IsAttached && !BallActor->IsMoving && ActivePlayers.Num() == 2)
	{
		// set up an array of actors
		for (AMyPaperCharacter* ActivePlayer : ActivePlayers)
		{
			TArray<AActor*> OverlapActors;
			if (ActivePlayer) ActivePlayer->GetOverlappingActors(OverlapActors, ABallActor::StaticClass());
			if (!OverlapActors.IsEmpty())
			{
				HoldingPlayer = ActivePlayer;
				HoldingPlayer->IsHolding = true;
				BallActor->AttachToActor(HoldingPlayer, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

				// TODO: Probably inefficient to loop through the array just to assign the NonHoldingPlayer
				// but less of a priority since
				for (AMyPaperCharacter* APlayerActor : ActivePlayers)
				{
					if (APlayerActor == HoldingPlayer) continue;
					else
					{
						NonHoldingPlayer = APlayerActor;
					}
				}
				BallActor->CanPass = true;
				BallActor->IsAttached = true;
			}
		}
	}
}

void AController2D::BallPassingHandler(float DeltaSeconds)
{
	// When the ball is passed and while it is traveling, this gets called every frame

	if (HasAuthority())
	{
		if (BallActor && BallActor->IsMoving && ActivePlayers.Num() == 2 && HoldingPlayer && NonHoldingPlayer)
		{
			// TODO: This should be moved to OnOverlapBegin - inefficient to constantly check an array of overlap actors
			TArray<AActor*> OverlapActors;
			NonHoldingPlayer->GetOverlappingActors(OverlapActors, ABallActor::StaticClass());
			if (!OverlapActors.IsEmpty())
			{

				// The ball has arrived at the NonHoldingPlayer
				NonHoldingPlayer->ResetJumpAbility(); // resets the jump for the player if they are in mid-air (core mechanic)
				BallActor->CanPass = true;
				BallActor->IsAttached = true;
				AMyPaperCharacter* TempPlayer = HoldingPlayer;
				HoldingPlayer = NonHoldingPlayer;
				NonHoldingPlayer = TempPlayer;
				HoldingPlayer->IsHolding = true;
				HoldingPlayer->RemoveBallArrivingWidget();
				BallActor->AttachToActor(HoldingPlayer, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				BallActor->IsMoving = false;

				BallActor->ForceNetUpdate();
			}
			else
			{
				FVector NewLocation = FMath::VInterpConstantTo(BallActor->GetActorLocation(), NonHoldingPlayer->GetActorLocation(), DeltaSeconds, BallActor->BallMovementSpeed); // this takes in 2 vectors and moves towards the target location at the given speed - this specifically returns the new location we need to be at
				BallActor->SetActorLocation(NewLocation);
			}
		}
	}
}

void AController2D::GatherActorsHandler()
{
	// This gathers the both the players and the ball actor into arrays that we use for passing and pickup
	// The reason this is not in BeginPlay() is because for now the players spawn into the level at separate times
	// So we'll just check every frame until both players are in
	if (ActivePlayers.Num() < 2)
	{
		TArray<AActor*> PaperActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), "Player", PaperActors);
		if (!PaperActors.IsEmpty())
		{
			// add players
			for (AActor* Actor : PaperActors) {
				AMyPaperCharacter* ActivePlayer = Cast<AMyPaperCharacter>(Actor);
				if (ActivePlayers.Find(ActivePlayer) == -1)
				{
					// add player if it's not already in our array
					ActivePlayers.Add(ActivePlayer);
				}
			}
		}
	}
	if (ActivePlayers.Num() == 2 && !PlayersSet)
	{
		for (AMyPaperCharacter* Actor : ActivePlayers)
		{
			if (Actor)
			{
				Actor->OnPassActivated.AddDynamic(this, &AController2D::OnPassActorActivated);
				Actor->OnActorBeginOverlap.AddDynamic(this, &AController2D::OnOverlapBegin);
			}
		}
		PlayersSet = true;
	}
	// TODO: can just move this to BeginPlay() whenever since the ball is just an object in the level
	if (!BallActor)
	{
		TArray<AActor*> BallActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), "Ball", BallActors);
		if (!BallActors.IsEmpty())
		{
			BallActor = Cast<ABallActor>(BallActors[0]);
		}
	}
}

void AController2D::OnOverlapBegin(AActor *PlayerActor, AActor* OtherActor)
{
	// death handling
	if (OtherActor->ActorHasTag("Death"))
	{
		AMyPaperCharacter* PlayerCharacterActor = Cast<AMyPaperCharacter>(PlayerActor);
		if (PlayerCharacterActor)
		{
			// The player dies - returning them to spawn (or checkpoint) and disabling movement for a short amount of time
			if (PlayerCharacterActor)
			{
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
		for (AMyPaperCharacter* ActivePlayer : ActivePlayers)
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

void AController2D::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AController2D, HoldingPlayer);
	DOREPLIFETIME(AController2D, NonHoldingPlayer);
	DOREPLIFETIME(AController2D, ActivePlayers);
	DOREPLIFETIME(AController2D, BallActor);
}