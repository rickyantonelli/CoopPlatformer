// Copyright Ricky Antonelli

#include "Mechanics/Triggers/ToggleActorsTrigger.h"

AToggleActorsTrigger::AToggleActorsTrigger()
{
	TriggerAmount = 2; // efaulted to 2, but customizable in case we want the trigger to just be for 1 player
	CanTrigger = false;
}

void AToggleActorsTrigger::OnOverlapBegin(AActor* TriggerBoxActor, AActor* OtherActor)
{
	if (OtherActor->ActorHasTag("Player"))
	{
		if (PlayerActors.Find(OtherActor) == -1)
		{
			// add the player to the array if they arent already there
			PlayerActors.Add(OtherActor);
			if (PlayerActors.Num() >= TriggerAmount) CanTrigger = true;
		}

		if (CanTrigger && HasAuthority())
		{
			MulticastEnableActors();
			MulticastDisableActors();
		}
	}
}

void AToggleActorsTrigger::OnOverlapEnd(AActor* TriggerBoxActor, AActor* OtherActor)
{
	// if a player steps out before the event is triggered, remove from the array
	if (OtherActor->ActorHasTag("Player") && PlayerActors.Find(OtherActor) != -1)
	{
		PlayerActors.Remove(OtherActor);
	}
}

void AToggleActorsTrigger::MulticastDisableActors_Implementation()
{
	TArray<AActor*> DisableAttachedActors;
	DisableActor->GetAttachedActors(DisableAttachedActors);
	if (!DisableAttachedActors.IsEmpty())
	{
		for (AActor* AttachedActor : DisableAttachedActors)
		{
			// turn off the meshes for all actors that are to be disabled
			UStaticMeshComponent* ActorMesh = AttachedActor->GetComponentByClass<UStaticMeshComponent>();
			if (ActorMesh)
			{
				ActorMesh->SetVisibility(false);
				ActorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}

void AToggleActorsTrigger::MulticastEnableActors_Implementation()
{
	// this is meant to take an actor, and perform an action on all its children
	TArray<AActor*> EnableAttachedActors;
	EnableActor->GetAttachedActors(EnableAttachedActors);
	if (!EnableAttachedActors.IsEmpty())
	{
		for (AActor* AttachedActor : EnableAttachedActors)
		{
			// turn on the meshes for all actors that are to be enabled
			UStaticMeshComponent* ActorMesh = AttachedActor->GetComponentByClass<UStaticMeshComponent>();
			if (ActorMesh)
			{
				ActorMesh->SetVisibility(true);
				ActorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}
		}
	}
}

void AToggleActorsTrigger::BeginPlay()
{
	Super::BeginPlay();
	OnActorBeginOverlap.AddDynamic(this, &AToggleActorsTrigger::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &AToggleActorsTrigger::OnOverlapEnd);

	// turn off the collision of the enable actors
	// we do this here and not just in the editor so that we can see these meshes while developing
	TArray<AActor*> EnableAttachedActors;
	EnableActor->GetAttachedActors(EnableAttachedActors);
	if (!EnableAttachedActors.IsEmpty())
	{
		for (AActor* AttachedActor : EnableAttachedActors)
		{
			// turn on the meshes for all actors that are to be enabled
			UStaticMeshComponent* ActorMesh = AttachedActor->GetComponentByClass<UStaticMeshComponent>();
			if (ActorMesh)
			{
				ActorMesh->SetVisibility(false);
				ActorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}