// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Components/BoxComponent.h>
#include "PaperSpriteComponent.h"
#include "PressurePlate.generated.h"

/** PressurePlate is the class that allows players to step on a pressure plate and enable/disable another actor */
UCLASS()
class COOPPLATFORMER_API APressurePlate : public AActor
{
	GENERATED_BODY()
	
public:	
	/** Default constructor for APressurePlate - Sets components, replication, and initializes variables*/
	APressurePlate();

protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

	/** Required for replicated variables - required for passing between players */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	/** The root component of the pressure plate actor */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	/** The trigger mesh component of the pressure plate actor - which sits just above the static mesh*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> TriggerMesh;

	/** The pressure plate's static mesh*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UPaperSpriteComponent> Mesh;

	/** The actor that is influenced by the pressure plate change */
	UPROPERTY(EditAnywhere, Category = "Custom")
	TObjectPtr<AActor> PressurePlatedActor;

	UPROPERTY(EditAnywhere, Category = "Custom")
	float OffsetAmount;

	/** When the player collides, disables or enables the PressurePlated Actor */
	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** When collision with the player ends, reverts the PressurePlated Actor to what it was before */
	UFUNCTION()
	void OnBoxCollisionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Whether the pressure plate is activated */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool Activated;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStepOn();

};
