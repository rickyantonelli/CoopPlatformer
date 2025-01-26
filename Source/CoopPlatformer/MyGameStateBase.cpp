// Copyright Ricky Antonelli


#include "MyGameStateBase.h"
#include "Net/UnrealNetwork.h"

void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameStateBase, ActivePlayers);
}
