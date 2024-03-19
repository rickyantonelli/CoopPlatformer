// Fill out your copyright notice in the Description page of Project Settings.


#include "ToggleActorsTrigger.h"

AToggleActorsTrigger::AToggleActorsTrigger()
{
	TriggerAmount = 2;
	CanTrigger = false;
}

void AToggleActorsTrigger::OnOverlapBegin(AActor* TriggerBoxActor, AActor* OtherActor)
{
	// OtherActor has to be a player
	// Add OtherActor to the array
	// get the children of the actor
	if (OtherActor->ActorHasTag("Player"))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Player Entered");
		if (PlayerActors.Find(OtherActor) == -1)
		{
			PlayerActors.Add(OtherActor);
			if (PlayerActors.Num() >= TriggerAmount) CanTrigger = true;
		}

		if (CanTrigger)
		{
			TArray<AActor*> EnableAttachedActors;
			EnableActor->GetAttachedActors(EnableAttachedActors);
			if (!EnableAttachedActors.IsEmpty())
			{
				for (AActor* AttachedActor : EnableAttachedActors)
				{
					UStaticMeshComponent* ActorMesh = AttachedActor->GetComponentByClass<UStaticMeshComponent>();
					if (ActorMesh)
					{
						ActorMesh->SetVisibility(true);
						ActorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					}
				}
			}

			TArray<AActor*> DisableAttachedActors;
			DisableActor->GetAttachedActors(DisableAttachedActors);
			if (!DisableAttachedActors.IsEmpty())
			{
				for (AActor* AttachedActor : DisableAttachedActors)
				{
					UStaticMeshComponent* ActorMesh = AttachedActor->GetComponentByClass<UStaticMeshComponent>();
					if (ActorMesh)
					{
						ActorMesh->SetVisibility(false);
						ActorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					}
				}
			}
		}
	}
}

void AToggleActorsTrigger::OnOverlapEnd(AActor* TriggerBoxActor, AActor* OtherActor)
{
	if (OtherActor->ActorHasTag("Player") && PlayerActors.Find(OtherActor) != -1)
	{
		PlayerActors.Remove(OtherActor);
	}
}



void AToggleActorsTrigger::BeginPlay()
{
	Super::BeginPlay();
	OnActorBeginOverlap.AddDynamic(this, &AToggleActorsTrigger::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &AToggleActorsTrigger::OnOverlapEnd);

}