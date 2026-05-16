// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/ENovaLevel.h"
#include "LevelExit.generated.h"

class USceneComponent;
class UBoxComponent;
class AMyPaperCharacter;
class UUserWidget;

/**
 * ALevelExit — placed at the end of a level.
 * Requires both players to overlap the trigger box before initiating level travel.
 * Fires a multicast to all clients (for local celebration/UI), waits a configurable
 * delay, then the server calls GI->TravelToLevel.
 */
UCLASS()
class COOPPLATFORMER_API ALevelExit : public AActor
{
	GENERATED_BODY()

public:
	ALevelExit();

protected:
	virtual void BeginPlay() override;

public:
	// -------------------------------------------------------------------------
	// Components
	// -------------------------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> TriggerBox;

	// -------------------------------------------------------------------------
	// Configuration — set these per-instance in the level
	// -------------------------------------------------------------------------

	/** Level to travel to when both players overlap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customizable|Values")
	ENovaLevel NextLevel = ENovaLevel::World1;

	/** Seconds to wait after both players overlap before actually travelling. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customizable|Values")
	float TravelDelay = 2.5f;


	// -------------------------------------------------------------------------
	// State
	// -------------------------------------------------------------------------

	/** Players that have entered the trigger. Populated server-side only. */
	UPROPERTY(VisibleAnywhere, Category = "Customizable|Debug")
	TArray<TObjectPtr<AMyPaperCharacter>> ExitingPlayers;

	/** Prevents double-activation if both players are standing in the zone. */
	UPROPERTY(VisibleAnywhere, Category = "Customizable|Debug")
	bool bCanActivate = true;

private:
	// -------------------------------------------------------------------------
	// Overlap
	// -------------------------------------------------------------------------

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Called on all clients once both players are in the zone. Hook into this in BP for VFX/UI. */
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void MulticastLevelComplete();

	/** Server-only: multicasts the loading screen just before travel. */
	void ServerTriggerLoadingScreen();

	/** Server-only: fires GI->TravelToLevel after TravelDelay. */
	void ServerExecuteTravel();
};
