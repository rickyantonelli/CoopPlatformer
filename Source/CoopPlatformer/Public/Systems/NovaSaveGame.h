// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "NovaSaveGame.generated.h"

/** Per-level save data. Extend this struct as more per-level state is needed. */
USTRUCT(BlueprintType)
struct FLevelSaveData
{
	GENERATED_BODY()

	/** The highest CheckpointID reached in this level. -1 means the level has never been visited. */
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Progress")
	int32 HighestCheckpointID = -1;
};

/**
 * Save game object for CoopPlatformer.
 */
UCLASS()
class COOPPLATFORMER_API UNovaSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/**
	 * Key:   Level asset name (e.g. "MAP_World1")
	 * Value: Save data for that level
	 */
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Progress")
	TMap<FName, FLevelSaveData> LevelProgress;
};
