// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Mechanics/Keys/KeyActor.h"
#include "TimerKeyActor.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API ATimerKeyActor : public AKeyActor
{
	GENERATED_BODY()
	
public:
	ATimerKeyActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Override for Tick*/
	virtual void Tick(float DeltaSeconds) override;

protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

public:
	/** When the player collides, disables or enables the PressurePlated Actor */
	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<UBoxComponent*> KeyMeshes;

	bool InTimer;

	UPROPERTY(EditAnywhere)
	float UnlockTimer;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastTimerExpired();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastYellowKey(UPaperSpriteComponent* SpriteComp);

private:
	UPROPERTY(VisibleAnywhere, Replicated)
	TArray<UPrimitiveComponent*> OverlappedMeshes;
};
