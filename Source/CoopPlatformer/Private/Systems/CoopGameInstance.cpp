// Copyright Ricky Antonelli

#include "Systems/CoopGameInstance.h"
#include "Systems/NovaSaveGame.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Misc/PackageName.h"

const FString UCoopGameInstance::SaveSlotName = TEXT("CoopPlatformerSlot");

void UCoopGameInstance::Init()
{
	Super::Init();

	// When the client is disconnected (host destroyed session, connection lost, etc.),
	// automatically travel back to the main menu.
	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddLambda([this](UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
		{
			UE_LOG(LogTemp, Warning, TEXT("Network failure: %s — returning to main menu."), *ErrorString);

			APlayerController* PC = GetFirstLocalPlayerController();
			if (PC)
			{
				PC->ClientTravel(TEXT("/Game/Dynamic/MainMenu/MainMenu"), ETravelType::TRAVEL_Absolute);
			}
		});
	}
}

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

void UCoopGameInstance::TravelToLevel(ENovaLevel Level, int32 CheckpointID /* = -1 */, bool bAttachBall /* = false */)
{
	const FString Path = GetLevelPath(Level);
	if (Path.IsEmpty())
	{
		return;
	}

	PendingCheckpointID = CheckpointID;
	bPendingAttachBall  = bAttachBall;

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
