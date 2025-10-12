// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Mechanics/Keys/KeyActor.h"
#include "ExtendKeyActor.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API AExtendKeyActor : public AKeyActor
{
	GENERATED_BODY()
	
public:
	AExtendKeyActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	UPROPERTY(EditAnywhere, Category = "Customizable")
	float UnlockTimer;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastTimerExpired();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastYellowKey(UPaperSpriteComponent* SpriteComp);

private:
	UPROPERTY(VisibleAnywhere, Replicated)
	TArray<UPrimitiveComponent*> OverlappedMeshes;

	FTimerHandle UnlockTimerHandle;
};
