// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller2D.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

void AController2D::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GatherActorsHandler();
	BallPickupHandler();
	BallPassingHandler(DeltaSeconds);
}

void AController2D::BeginPlay()
{
	Super::BeginPlay();
}


void AController2D::OnPassActorActivated()
{
	PassServerRPCFunction();
}

void AController2D::BallPickupMulticastFunction_Implementation(AMyPaperCharacter* MyPlayerActor)
{
	if (HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Attached Server");
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Attached Client");
	}
	BallActor->AttachToComponent(MyPlayerActor->BallHolder, FAttachmentTransformRules::SnapToTargetNotIncludingScale); // attach this to the actor
	MyPlayerActor->IsHolding = true;
	HoldingPlayer = MyPlayerActor;
	for (AMyPaperCharacter* APlayerActor : ActivePlayers)
	{
		if (APlayerActor == HoldingPlayer) continue;
		else
		{
			NonHoldingPlayer = APlayerActor;
		}
	}
	BallActor->IsHeld = true;
	BallActor->CanPass = true;
	BallActor->IsAttached = true;
}

void AController2D::PassServerRPCFunction_Implementation()
{
	if (BallActor->CanPass && BallActor->NoPassCooldown && HoldingPlayer && NonHoldingPlayer && BallActor->GetAttachParentActor())
	{
		BallActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		BallActor->IsAttached = false;
		HoldingPlayer->IsHolding = false;
		BallActor->IsMoving = true;
		BallActor->CanPass = false;
		BallActor->BeginPassCooldown();
	}
	PassMulticastFunction();
}

void AController2D::PassMulticastFunction_Implementation()
{
	if (BallActor->CanPass && BallActor->NoPassCooldown && HoldingPlayer && NonHoldingPlayer && BallActor->GetAttachParentActor())
	{
		BallActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		BallActor->IsAttached = false;
		HoldingPlayer->IsHolding = false;
		BallActor->IsMoving = true;
		BallActor->CanPass = false;
		BallActor->BeginPassCooldown();
	}
}

void AController2D::GatherPlayersMulticastFunction_Implementation(const TArray<AMyPaperCharacter*>& UpdatedActivePlayers)
{
	for (int i = 0; i < ActivePlayers.Num(); i++)
	{
		ActivePlayers[i] = UpdatedActivePlayers[i];
	}
}

void AController2D::BallPickupHandler()
{
	if (BallActor && !BallActor->IsHeld && !BallActor->IsMoving && ActivePlayers.Num() == 2)
	{
		// set up an array of actors
		for (AMyPaperCharacter* ActivePlayer : ActivePlayers)
		{
			TArray<AActor*> OverlapActors;
			ActivePlayer->GetOverlappingActors(OverlapActors, ABallActor::StaticClass());
			if (!OverlapActors.IsEmpty())
			{
				if (HasAuthority()) BallPickupMulticastFunction(ActivePlayer);
			}
		}
	}
}

void AController2D::BallPassingHandler(float DeltaSeconds)
{
	if (BallActor && BallActor->IsMoving && ActivePlayers.Num() == 2 && HoldingPlayer && NonHoldingPlayer)
	{
		TArray<AActor*> OverlapActors;
		NonHoldingPlayer->BallHolder->GetOverlappingActors(OverlapActors, ABallActor::StaticClass());
		if (!OverlapActors.IsEmpty())
		{
			NonHoldingPlayer->ResetJumpAbility();
			BallActor->CanPass = true;
			BallActor->IsAttached = true;
			AMyPaperCharacter* TempPlayer = HoldingPlayer;
			HoldingPlayer = NonHoldingPlayer;
			NonHoldingPlayer = TempPlayer;
			BallActor->AttachToComponent(HoldingPlayer->BallHolder, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			BallActor->IsMoving = false;
		}
		else
		{
			FVector NewLocation = FMath::VInterpConstantTo(BallActor->GetActorLocation(), NonHoldingPlayer->BallHolder->GetComponentLocation(), DeltaSeconds, BallActor->BallMovementSpeed); // this takes in 2 vectors and moves towards the target location at the given speed - this specifically returns the new location we need to be at
			BallActor->SetActorLocation(NewLocation);
		}
	}
}

void AController2D::GatherActorsHandler()
{
	if (ActivePlayers.Num() < 2)
	{
		TArray<AActor*> PaperActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), "Player", PaperActors);
		if (!PaperActors.IsEmpty())
		{
			for (AActor* Actor : PaperActors) {
				AMyPaperCharacter* ActivePlayer = Cast<AMyPaperCharacter>(Actor);
				ActivePlayers.Add(ActivePlayer);
			}
			for (AMyPaperCharacter* Actor : ActivePlayers)
			{
				Actor->OnPassActivated.AddDynamic(this, &AController2D::OnPassActorActivated);
				Actor->OnActorBeginOverlap.AddDynamic(this, &AController2D::OnOverlapBegin);
			}
			if (HasAuthority()) GatherPlayersMulticastFunction(ActivePlayers);
		}
	}
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

void AController2D::PlayerDeathMulticastFunction_Implementation(AMyPaperCharacter* PlayerActor)
{
	if (HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Death Server");
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Death Client");
	}
	if (PlayerActor)
	{
		UCharacterMovementComponent* MyCharacterMovement = PlayerActor->GetCharacterMovement();
		if (MyCharacterMovement)
		{
			MyCharacterMovement->Velocity = FVector::ZeroVector;
		}
		PlayerActor->SetActorLocation(PlayerActor->SpawnLocation);
		PlayerActor->OnDeath();
	}
}

void AController2D::OnOverlapBegin(AActor *PlayerActor, AActor* OtherActor)
{

	// ball pickup handling
	/*if (OtherActor->ActorHasTag("Ball") && !BallActor->IsHeld && !BallActor->IsMoving && ActivePlayers.Num() == 2 && HasAuthority())
	{
		AMyPaperCharacter* PlayerCharacterActor = Cast<AMyPaperCharacter>(PlayerActor);
		if (PlayerCharacterActor) BallPickupMulticastFunction(PlayerCharacterActor);
	}*/

	// death handling
	if (OtherActor->ActorHasTag("Death") && HasAuthority())
	{
		AMyPaperCharacter* PlayerCharacterActor = Cast<AMyPaperCharacter>(PlayerActor);
		if (PlayerCharacterActor) PlayerDeathMulticastFunction(PlayerCharacterActor);
	}

	// checkpoint handling
	if (OtherActor->ActorHasTag("Checkpoint") && HasAuthority())
	{
		AMyPaperCharacter* PlayerCharacterActor = Cast<AMyPaperCharacter>(PlayerActor);
		ACheckpoint* CheckpointActor = Cast<ACheckpoint>(OtherActor);
		if (PlayerCharacterActor && CheckpointActor) CheckpointActivatedMulticastFunction(PlayerCharacterActor, CheckpointActor);
	}
}

void AController2D::CheckpointActivatedMulticastFunction_Implementation(AMyPaperCharacter* PlayerActor, ACheckpoint* Checkpoint)
{
	if (Checkpoint->CheckpointedPlayers.Find(PlayerActor) == -1) // if the player actor is not in our Tarray for the checkpoint
	{
		Checkpoint->AddPlayer(PlayerActor);
	}
}