// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller2D.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

void AController2D::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GatherActorsHandler();
	// BallPickupHandler();
	BallPassingHandler(DeltaSeconds);
	DeathHandler();
}


void AController2D::OnPassActorActivated()
{
	PassServerRPCFunction();
}

void AController2D::BallPickupMulticastFunction_Implementation(AMyPaperCharacter* MyPlayerActor)
{
	BallActor->AttachToComponent(MyPlayerActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale); // attach this to the actor
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

void AController2D::BallThrownServerRPCFunction_Implementation(float DeltaTime)
{
	if (ActivePlayers.Num() == 2)
	{
		if (BallActor->GetAttachParentActor())
		{
			BallActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			BallActor->IsAttached = false;
		}
		FVector CurrentLocation = HoldingPlayer->GetActorLocation();
		FVector TargetLocation = NonHoldingPlayer->GetActorLocation();
		TArray<AActor*> OverlapActors;
		NonHoldingPlayer->GetOverlappingActors(OverlapActors, ABallActor::StaticClass());
		if (!OverlapActors.IsEmpty())
		{
			NonHoldingPlayer->ResetJumpAbility();
			BallActor->IsMoving = false;
			BallActor->CanPass = true;
			BallActor->IsAttached = true;
			AMyPaperCharacter* TempPlayer = HoldingPlayer;
			HoldingPlayer = NonHoldingPlayer;
			NonHoldingPlayer = TempPlayer;
			BallActor->AttachToComponent(HoldingPlayer->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			if (!HoldingPlayer->IsHolding)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "It broke!");
			}
		}
		else
		{
			FVector NewLocation = FMath::VInterpConstantTo(BallActor->GetActorLocation(), TargetLocation, DeltaTime, BallActor->BallMovementSpeed); // this takes in 2 vectors and moves towards the target location at the given speed - this specifically returns the new location we need to be at
			BallActor->SetActorLocation(NewLocation);
			HoldingPlayer->IsHolding = false;
		}
	}

	BallThrownMulticastFunction(DeltaTime);
}

void AController2D::BallThrownMulticastFunction_Implementation(float DeltaTime)
{
	if (ActivePlayers.Num() == 2)
	{

		FVector CurrentLocation = HoldingPlayer->GetActorLocation();
		FVector TargetLocation = NonHoldingPlayer->GetActorLocation();
		TArray<AActor*> OverlapActors;
		NonHoldingPlayer->GetOverlappingActors(OverlapActors, ABallActor::StaticClass());
		if (!OverlapActors.IsEmpty())
		{
			NonHoldingPlayer->ResetJumpAbility();
			BallActor->CanPass = true;
			BallActor->IsAttached = true;
			AMyPaperCharacter* TempPlayer = HoldingPlayer;
			HoldingPlayer = NonHoldingPlayer;
			NonHoldingPlayer = TempPlayer;
			BallActor->AttachToComponent(HoldingPlayer->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			BallActor->IsMoving = false;
		}
		else
		{
			if (BallActor->GetAttachParentActor())
			{
				BallActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				BallActor->IsAttached = false;
				HoldingPlayer->IsHolding = false;
			}
			FVector NewLocation = FMath::VInterpConstantTo(BallActor->GetActorLocation(), TargetLocation, DeltaTime, BallActor->BallMovementSpeed); // this takes in 2 vectors and moves towards the target location at the given speed - this specifically returns the new location we need to be at
			BallActor->SetActorLocation(NewLocation);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Not enough");
	}
}

void AController2D::PassServerRPCFunction_Implementation()
{
	if (BallActor->CanPass)
	{
		if (HoldingPlayer && NonHoldingPlayer)
		{
			if (BallActor->GetAttachParentActor())
			{
				BallActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				BallActor->IsAttached = false;
				HoldingPlayer->IsHolding = false;
				BallActor->IsMoving = true;
				BallActor->CanPass = false;
			}
		}
		PassMulticastFunction();
	}
}

void AController2D::PassMulticastFunction_Implementation()
{
	if (BallActor->CanPass)
	{
		if (HoldingPlayer && NonHoldingPlayer)
		{
			if (BallActor->GetAttachParentActor())
			{
				BallActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				BallActor->IsAttached = false;
				HoldingPlayer->IsHolding = false;
				BallActor->IsMoving = true;
				BallActor->CanPass = false;
			}
		}
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
	if (!BallActor->IsHeld && !BallActor->IsMoving && ActivePlayers.Num() == 2)
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
	if (BallActor->IsMoving)
	{
		// lets move it to the other player
		if (ActivePlayers.Num() == 2)
		{
			if (HoldingPlayer && NonHoldingPlayer)
			{
				TArray<AActor*> OverlapActors;
				NonHoldingPlayer->GetOverlappingActors(OverlapActors, ABallActor::StaticClass());
				if (!OverlapActors.IsEmpty())
				{
					NonHoldingPlayer->ResetJumpAbility();
					BallActor->CanPass = true;
					BallActor->IsAttached = true;
					AMyPaperCharacter* TempPlayer = HoldingPlayer;
					HoldingPlayer = NonHoldingPlayer;
					NonHoldingPlayer = TempPlayer;
					BallActor->AttachToComponent(HoldingPlayer->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
					BallActor->IsMoving = false;
				}
				else
				{
					FVector NewLocation = FMath::VInterpConstantTo(BallActor->GetActorLocation(), NonHoldingPlayer->GetActorLocation(), DeltaSeconds, BallActor->BallMovementSpeed); // this takes in 2 vectors and moves towards the target location at the given speed - this specifically returns the new location we need to be at
					BallActor->SetActorLocation(NewLocation);
				}
			}
		}
	}
}

void AController2D::GatherActorsHandler()
{
	if (ActivePlayers.Num() < 2)
	{
		TArray<AActor*> PaperActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), "Player", PaperActors);
		for (AActor* Actor : PaperActors) {
			AMyPaperCharacter* ActivePlayer = Cast<AMyPaperCharacter>(Actor);
			ActivePlayers.Add(ActivePlayer);
		}
		for (AMyPaperCharacter* Actor : ActivePlayers)
		{
			Actor->OnPassActivated.AddDynamic(this, &AController2D::OnPassActorActivated);
			Actor->OnActorBeginOverlap.AddDynamic(this, &AController2D::OnOverlapBegin);
			// DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams( FActorBeginOverlapSignature, AActor, OnActorBeginOverlap, AActor*, OverlappedActor, AActor*, OtherActor );
		}
		if (HasAuthority()) GatherPlayersMulticastFunction(ActivePlayers);
	}
	if (!BallActor)
	{
		TArray<AActor*> BallActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), "Ball", BallActors);
		BallActor = Cast<ABallActor>(BallActors[0]);
	}
}

void AController2D::DeathHandler()
{
	

	// we tried the delegate idea and failed, so lets just do the normal first implementation with the "death" tag
	// we can probably do the same logic as ball pick (in terms of authority)
	for (AMyPaperCharacter* AA : ActivePlayers)
	{
		TArray<AActor*> OverlapActors;
		AA->GetOverlappingActors(OverlapActors);
		if (!OverlapActors.IsEmpty()) 
		{
			
			for (AActor* OverlapActor : OverlapActors)
			{
				if (OverlapActor->ActorHasTag("Death"))
				{
					//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Death");
					//AMyPaperCharacter* PlayerActor = Cast<AMyPaperCharacter>(OverlapActor);
					//if (PlayerActor && HasAuthority()) PlayerDeathMulticastFunction(PlayerActor);
				}
			}
		}
	}
}

void AController2D::PlayerDeathMulticastFunction_Implementation(AMyPaperCharacter* PlayerActor)
{
	// in this function we should keep Overlapping Actors, if any of them have type "death" then we know we died
	// when a player dies, they should respawn at the start or checkpoint
	// the ball should go back to the non-dead player, here's how we do this:
	// if IsMoving is false and the dead player is not the holder, do nothing
	// if IsMoving is false and the dead player is the holder, then pass to the other player
	// if IsMoving is true, and the dead player is NonHolding player, then swap Holding and Nonholding players so that (hopefully) the ball just turns around
	// if IsMoving is true, and the dead player is the Holding player, do nothing
	// for now lets just get a print statement
	

	// first we need to figure out if the player is the holdingcharacter
	if (PlayerActor == HoldingPlayer)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Death");
		PlayerActor->SetActorLocation(PlayerActor->SpawnLocation);// , false);// , (FHitResult*)nullptr, ETeleportType::ResetPhysics);
	}


	// once the ball is thrown, move the actor to the starting location
	

}

void AController2D::OnOverlapBegin(AActor *PlayerActor, AActor* OtherActor)
{

	// ball pickup handling
	if (!BallActor->IsHeld && !BallActor->IsMoving && ActivePlayers.Num() == 2 && OtherActor->ActorHasTag("Ball") && HasAuthority())
	{
		AMyPaperCharacter* PlayerCharacterActor = Cast<AMyPaperCharacter>(PlayerActor);
		if (PlayerCharacterActor) BallPickupMulticastFunction(PlayerCharacterActor);
	}

	// death handling
	if (OtherActor->ActorHasTag("Death") && HasAuthority())
	{
		AMyPaperCharacter* PlayerCharacterActor = Cast<AMyPaperCharacter>(PlayerActor);
		if (PlayerCharacterActor) PlayerDeathMulticastFunction(PlayerCharacterActor);
	}
}