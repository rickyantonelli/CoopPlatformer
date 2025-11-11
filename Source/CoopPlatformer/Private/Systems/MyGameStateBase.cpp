// Copyright Ricky Antonelli


#include "Systems/MyGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameStateBase, ActivePlayers);
	// DOREPLIFETIME(AMyGameStateBase, ActiveControllers);
	DOREPLIFETIME(AMyGameStateBase, BallActor);
}

void AMyGameStateBase::MulticastPlayPassSound_Implementation(USoundBase* Sound)
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("Playing sound server"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Playing sound client"));
	}
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}
