// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "KeyActor.h"
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

	virtual void MulticastTriggerUnlock_Implementation() override;

public:
	/** The key's static mesh*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated)
	TObjectPtr<UBoxComponent> Mesh;

	/** When the player collides, disables or enables the PressurePlated Actor */
	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
