// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Mechanics/Keys/KeyActor.h"
#include "PassKeyActor.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API APassKeyActor : public AKeyActor
{
	GENERATED_BODY()

public:
	APassKeyActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UPaperSpriteComponent> Sprite;

	/** When the player collides, disables or enables the PressurePlated Actor */
	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastYellowKey(UPaperSpriteComponent* SpriteComp);
};
