// Copyright Ricky Antonelli


#include "Systems/MyPlayerState.h"
#include "Net/UnrealNetwork.h"

AMyPlayerState::AMyPlayerState()
{
    // for simultaneous spawning after all players have loaded into the game
	IsLoaded = false;
}

void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMyPlayerState, IsLoaded);
}
