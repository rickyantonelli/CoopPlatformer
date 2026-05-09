// Copyright Ricky Antonelli

#include "Systems/CoopGameInstance.h"
#include "Systems/NovaSaveGame.h"
#include "Engine/World.h"
#include "Misc/PackageName.h"

const FString UCoopGameInstance::SaveSlotName = TEXT("CoopPlatformerSlot");

FString UCoopGameInstance::GetLevelPath(ENovaLevel Level) const
{
	const FNovaLevelEntry* Found = LevelMap.Find(Level);
	if (!Found || Found->MapAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetLevelPath: ENovaLevel %d is not configured in BP_GameInstance's LevelMap."), static_cast<int32>(Level));
		return FString();
	}

	// ServerTravel expects a package path like "/Game/Dynamic/Maps/MAP_World1".
	return Found->MapAsset.GetLongPackageName();
}

void UCoopGameInstance::TravelToLevel(ENovaLevel Level, int32 CheckpointID /* = -1 */)
{
	const FString Path = GetLevelPath(Level);
	if (Path.IsEmpty())
	{
		return;
	}

	PendingCheckpointID = CheckpointID;

	// Initialize save entry for this level if it doesn't exist yet
	if (bSavingEnabled && CurrentSaveGame)
	{
		const FName LevelName = FName(*FPackageName::GetShortName(Path));
		const bool bWasNew = !CurrentSaveGame->LevelProgress.Contains(LevelName);
		CurrentSaveGame->LevelProgress.FindOrAdd(LevelName);
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FString TravelURL = FString::Printf(TEXT("%s?listen"), *Path);
	World->ServerTravel(TravelURL);
}
