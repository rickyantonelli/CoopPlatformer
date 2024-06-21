// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPaperCharacter.h"
#include "BallActor.h"
#include "Checkpoint.h"


#include "Controller2D.generated.h"

/**
 *
 */
UCLASS()
class COOPPLATFORMER_API AController2D : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	AMyPaperCharacter* HoldingPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	AMyPaperCharacter* NonHoldingPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TArray<AMyPaperCharacter*> ActivePlayers;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	ABallActor* BallActor;

	UFUNCTION()
	void OnPassActorActivated(AMyPaperCharacter* PassingPlayer);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void PassServerRPCFunction();

	UFUNCTION()
	void DeferredDetachBall();

	UFUNCTION()
	void OnOverlapBegin(AActor* PlayerActor, AActor* OtherActor);

	void GatherActorsHandler();

	void BallPickupHandler();

	void BallPassingHandler(float DeltaSeconds);

	bool PlayersSet;
};