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
	
protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

public:
	/** The key's static mesh*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** When the player collides, disables or enables the PressurePlated Actor */
	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
