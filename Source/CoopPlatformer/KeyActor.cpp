// Copyright Ricky Antonelli


#include "KeyActor.h"
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
	//
}

