// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "ENovaLevel.generated.h"

/**
 * Type-safe identifiers for every travelable level in Nova.
 * Add a new entry here when adding a new level, then populate the
 * corresponding entry in BP_GameInstance's LevelMap.
 */
UENUM(BlueprintType)
enum class ENovaLevel : uint8
{
	MainMenu    UMETA(DisplayName = "Main Menu"),
	WaitingArea UMETA(DisplayName = "Waiting Area"),
	World1      UMETA(DisplayName = "World 1"),
};
