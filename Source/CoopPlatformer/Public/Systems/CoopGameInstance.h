// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Systems/ENovaLevel.h"
#include "CoopGameInstance.generated.h"

class UNovaSaveGame;
class UNovaLevelData;

// ---------------------------------------------------------------------------
// Per-level registry entry — populate in BP_GameInstance's Class Defaults
// ---------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FNovaLevelEntry
{
	GENERATED_BODY()

	/** The UWorld asset for this level. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TSoftObjectPtr<UWorld> MapAsset;

	/** Display name shown in the main menu (e.g. "World 1"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	FText DisplayName;

	/** Baked checkpoint data for this level. Assign after running BakeCheckpointData. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TObjectPtr<UNovaLevelData> LevelData;
};

/**
 * Game instance for CoopPlatformer.
 * Hosts the save/load entry points, the active save game object,
 * and the level registry for type-safe level travel.
 */
UCLASS()
class COOPPLATFORMER_API UCoopGameInstance : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;


public:
	/** Slot name used for SaveGameToSlot / LoadGameFromSlot. */
	static const FString SaveSlotName;

	/** If false, all save/load operations are skipped. Toggle in BP_GameInstance's Class Defaults for testing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	bool bSavingEnabled = true;

	/** User index used for SaveGameToSlot / LoadGameFromSlot. */
	static constexpr int32 SaveUserIndex = 0;

	/** Currently loaded / in-memory save game object. */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	TObjectPtr<UNovaSaveGame> CurrentSaveGame;

	// -------------------------------------------------------------------------
	// Level Registry
	// -------------------------------------------------------------------------

	/**
	 * Maps each ENovaLevel to its entry (world asset, display name, checkpoint data).
	 * Populate this in BP_GameInstance's Class Defaults.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Travel")
	TMap<ENovaLevel, FNovaLevelEntry> LevelMap;

	/**
	 * Resolves a level enum to its package path (e.g. "/Game/Dynamic/Maps/MAP_World1").
	 * Returns an empty string if the level is not in LevelMap.
	 */
	UFUNCTION(BlueprintPure, Category = "Travel")
	FString GetLevelPath(ENovaLevel Level) const;

	/**
	 * Server-travels both players to the given level immediately.
	 * Use this for in-game transitions (checkpoints, level completion, etc.).
	 * For the main-menu flow, use GetLevelPath + MultiplayerSessionsSubsystem::CreateServer instead.
	 */
	/**
	 * Server-travels both players to the given level.
	 * If CheckpointID > 1, the GameMode will teleport all players to that checkpoint
	 * after they finish loading, before the loading screen is dropped.
	 * Pass INDEX_NONE (or omit) to skip checkpoint teleport.
	 */
	UFUNCTION(BlueprintCallable, Category = "Travel")
	void TravelToLevel(ENovaLevel Level, int32 CheckpointID = -1, bool bAttachBall = false);

	/**
	 * Set by TravelToLevel before ServerTravel fires.
	 * Read and cleared by CheckAllPlayersLoaded after the teleport runs.
	 * INDEX_NONE means no checkpoint teleport was requested.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Travel")
	int32 PendingCheckpointID = -1;

	/**
	 * Set by TravelToLevel before ServerTravel fires.
	 * Read and cleared by CheckAllPlayersLoaded to give the ball to the host.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Travel")
	bool bPendingAttachBall = false;
};
