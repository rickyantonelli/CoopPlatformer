// Copyright Ricky Antonelli


#include "KeyActor.h"
#include "PaperSpriteComponent.h"
#include "Net/UnrealNetwork.h"

AKeyActor::AKeyActor()
{
	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Locked = true;

}

void AKeyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKeyActor, LockedActors);
}

void AKeyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AKeyActor::MulticastTriggerUnlock_Implementation()
{
	Locked = false;
	for (AActor* LockedActor : LockedActors)
	{
		// for a while the implementation will be as such - to keep flexibility
		// first check if there is a static mesh, if there is then just set the visibility of the mesh and collision off
		// if there is no mesh, then we are dealing with a sprite - which is going to have a box component and PaperSprite component


		// set this to false so that we dont check overlaps anymore since it's already been unlocked
		UStaticMeshComponent* LockMesh = LockedActor->GetComponentByClass<UStaticMeshComponent>();
		if (LockMesh)
		{
			LockMesh->SetVisibility(false);
			LockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			UBoxComponent* LockBox = LockedActor->GetComponentByClass<UBoxComponent>();
			UPaperSpriteComponent* LockSprite = LockedActor->GetComponentByClass<UPaperSpriteComponent>();
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
}

