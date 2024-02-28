// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller2D.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

void AController2D::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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
		}
		if (HasAuthority()) GatherPlayersMulticastFunction(ActivePlayers);
	}
	if (!BallActor)
	{
		TArray<AActor*> BallActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), "Ball", BallActors);
		BallActor = Cast<ABallActor>(BallActors[0]);
	}


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


void AController2D::OnPassActorActivated()
{
	PassServerRPCFunction();
}

void AController2D::AttachBallToPlayer()
{
	TArray<AActor*> OverlapActors; // set up an array of actors
	BallActor->Sphere->GetOverlappingActors(OverlapActors, AMyPaperCharacter::StaticClass()); // overlapping actors that are of the character class

	if (!OverlapActors.IsEmpty())
	{

		// make the first actor in the array the parent of this actor
		AMyPaperCharacter* HoldingPlayerActor = nullptr;
		for (AActor* OverlappingActor : OverlapActors)
		{
			HoldingPlayerActor = Cast<AMyPaperCharacter>(OverlappingActor);
			break;
		}
		if (HoldingPlayerActor) // && !HasAuthority())
		{
			BallActor->AttachToComponent(HoldingPlayerActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale); // attach this to the actor
			HoldingPlayerActor->IsHolding = true;
			HoldingPlayer = HoldingPlayerActor;
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
		}
	}
}

void AController2D::BallPickupServerRPCFunction_Implementation(AMyPaperCharacter* MyPlayerActor)
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
	BallPickupMulticastFunction(MyPlayerActor);
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

void AController2D::GatherPlayersServerRPCFunction_Implementation(const TArray<AMyPaperCharacter*>& UpdatedActivePlayers)
{
	//
}

void AController2D::GatherPlayersMulticastFunction_Implementation(const TArray<AMyPaperCharacter*>& UpdatedActivePlayers)
{
	for (int i = 0; i < ActivePlayers.Num(); i++)
	{
		ActivePlayers[i] = UpdatedActivePlayers[i];
	}
}