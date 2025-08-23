// Copyright Ricky Antonelli


#include "Mechanics/Triggers/ActorTriggerArea.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"
#include "Net/UnrealNetwork.h"

AActorTriggerArea::AActorTriggerArea()
{
	bReplicates = true;
	SetReplicateMovement(true);
	// Set to 2 here, but there will be times where we want this number to be 1 so we'll leave it open
	TriggerAmount = 2;

	// I'll rework this later, but this is just a bool to give flexibility on the blueprint side
	// If true, it means we are enabling actors on event, if false vice versa
	EnableChoice = false;
	bUnlocked = true;
}

void AActorTriggerArea::BeginPlay()
{
	Super::BeginPlay();
	OnActorBeginOverlap.AddDynamic(this, &AActorTriggerArea::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &AActorTriggerArea::OnOverlapEnd);

	if (EnableChoice)
	{
		// if it's an enable trigger, then we want to disable these on beginplay
		UBoxComponent* LockBox = EnableActor->GetComponentByClass<UBoxComponent>();
		UPaperSpriteComponent* LockSprite = EnableActor->GetComponentByClass<UPaperSpriteComponent>();
		if (LockBox)
		{
			LockBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (LockSprite)
		{
			LockSprite->SetVisibility(false);
		}
	}
}

void AActorTriggerArea::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AActorTriggerArea, bUnlocked);
}

void AActorTriggerArea::OnOverlapBegin(AActor* TriggerBoxActor, AActor* OtherActor)
{
	if (OtherActor->ActorHasTag("Player") && PlayerActors.Find(OtherActor) == -1 && bUnlocked && HasAuthority())
	{
		// add the player to the array if they arent already there
		PlayerActors.Add(OtherActor);
		if (PlayerActors.Num() >= TriggerAmount)
		{
			bUnlocked = false;
			if (EnableChoice)
			{
				MulticastEnableActor();
			}
			else
			{
				MulticastDisableActor();
			}
		}
	}
}

void AActorTriggerArea::OnOverlapEnd(AActor* TriggerBoxActor, AActor* OtherActor)
{
	// if a player steps out before the event is triggered, remove from the array
	if (OtherActor->ActorHasTag("Player") && PlayerActors.Find(OtherActor) != -1)
	{
		PlayerActors.Remove(OtherActor);
	}
}

void AActorTriggerArea::MulticastDisableActor_Implementation()
{
	if (!DisableActor) return;
	UBoxComponent* LockBox = DisableActor->GetComponentByClass<UBoxComponent>();
	UPaperSpriteComponent* LockSprite = DisableActor->GetComponentByClass<UPaperSpriteComponent>();
	if (LockBox)
	{
		LockBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (LockSprite)
	{
		LockSprite->SetVisibility(false);
	}
}

void AActorTriggerArea::MulticastEnableActor_Implementation()
{
	UBoxComponent* LockBox = EnableActor->GetComponentByClass<UBoxComponent>();
	UPaperSpriteComponent* LockSprite = EnableActor->GetComponentByClass<UPaperSpriteComponent>();
	if (LockBox)
	{
		LockBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	if (LockSprite)
	{
		LockSprite->SetVisibility(true);
	}
}
