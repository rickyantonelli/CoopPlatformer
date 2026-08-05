// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Mechanics/Keys/KeyActor.h"
#include "Controller/Controller2D.h"
#include "SamePassKeyActor.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API ASamePassKeyActor : public AKeyActor
{
	GENERATED_BODY()

public:
	ASamePassKeyActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Override for Tick*/
	virtual void Tick(float DeltaSeconds) override;

protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

	virtual void MulticastTriggerUnlock_Implementation() override;

public:
	/** When the player collides, disables or enables the PressurePlated Actor */
	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBallCaught();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<UPaperSpriteComponent*> KeyMeshes;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastYellowKey(UPaperSpriteComponent* SpriteComp, UPaperFlipbookComponent* FlipbookComp);

	UFUNCTION()
	void OnYellowFlipbookFinished();

	UFUNCTION()
	void OnSamePassUnlockFlipbookFinished();

private:
	UPaperFlipbookComponent* FindFlipbookForSprite(UPaperSpriteComponent* SpriteComp) const;

	UPROPERTY(VisibleAnywhere, Replicated)
	TArray<UPrimitiveComponent*> OverlappedMeshes;

	TMap<UPaperFlipbookComponent*, UPaperSpriteComponent*> PendingYellowTransitions;
	TMap<UPaperFlipbookComponent*, UPaperSpriteComponent*> PendingUnlockTransitions;
	
};
