// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Mechanics/Keys/KeyActor.h"
#include "ResetKeyActor.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API AResetKeyActor : public AKeyActor
{
	GENERATED_BODY()

public:
	AResetKeyActor();
	
protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

public:
	/** The key's static mesh*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UBoxComponent> Mesh;

	/** When the player collides, disables or enables the PressurePlated Actor */
	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ResetTimer = 3.0f;

};
