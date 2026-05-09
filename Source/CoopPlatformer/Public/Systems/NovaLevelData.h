// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NovaLevelData.generated.h"

/**
 * Baked per-level checkpoint registry.
 * Populated by the BakeCheckpointData editor script.
 * One asset per level (e.g. DA_World1), assigned in BP_GameInstance's LevelMap.
 */
UCLASS(BlueprintType)
class COOPPLATFORMER_API UNovaLevelData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * All CheckpointIDs present in this level, sorted ascending.
	 * Baked by the BakeCheckpointData editor script - do not edit by hand.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Checkpoints")
	TArray<int32> CheckpointIDs;
};
