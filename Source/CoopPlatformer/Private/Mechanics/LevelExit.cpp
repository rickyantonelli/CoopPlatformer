// Copyright Ricky Antonelli

#include "Mechanics/LevelExit.h"
#include "Character/MyPaperCharacter.h"
#include "Systems/CoopGameInstance.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"


ALevelExit::ALevelExit()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComp);
	TriggerBox->SetCollisionProfileName(FName("OverlapAllDynamic"));
	TriggerBox->SetIsReplicated(true);
}

void ALevelExit::BeginPlay()
{
	Super::BeginPlay();

	// Only the server needs to process overlaps — clients just receive the multicast.
	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ALevelExit::OnTriggerBeginOverlap);
	}
}

void ALevelExit::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bCanActivate) return;

	AMyPaperCharacter* Player = Cast<AMyPaperCharacter>(OtherActor);
	if (!Player) return;

	// Deduplicate — same player can fire multiple overlap events
	if (ExitingPlayers.Contains(Player)) return;

	ExitingPlayers.Add(Player);

	if (ExitingPlayers.Num() >= 2)
	{
		bCanActivate = false;

		// Show loading screen on all clients just before travel fires
		FTimerHandle LoadingScreenTimerHandle;
		const float LoadingScreenDelay = FMath::Max(0.f, TravelDelay - 0.5f);
		GetWorldTimerManager().SetTimer(LoadingScreenTimerHandle, this, &ALevelExit::ServerTriggerLoadingScreen, LoadingScreenDelay, false);

		// Start the countdown to travel on the server
		FTimerHandle TravelTimerHandle;
		GetWorldTimerManager().SetTimer(TravelTimerHandle, this, &ALevelExit::ServerExecuteTravel, TravelDelay, false);
	}
}

void ALevelExit::ServerTriggerLoadingScreen()
{
	MulticastLevelComplete();
}

void ALevelExit::MulticastLevelComplete_Implementation()
{
	// Each machine disables movement on the pawn it locally owns.
	// ExitingPlayers is only populated on the server, so we can't iterate it here.
	APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController(GetWorld());
	if (PC)
	{
		if (AMyPaperCharacter* Player = Cast<AMyPaperCharacter>(PC->GetPawn()))
		{
			Player->ShowLoadingScreen(); // Also sets MovementEnabled = false internally
		}
	}
}

void ALevelExit::ServerExecuteTravel()
{
	if (!HasAuthority())
	{
		return;
	}

	UCoopGameInstance* GI = GetGameInstance<UCoopGameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("ALevelExit::ServerExecuteTravel — CoopGameInstance not found."));
		return;
	}

	GI->TravelToLevel(NextLevel);
}
