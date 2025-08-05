// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/Actor.h"
#include "ToggleActorsTrigger.generated.h"

/** The ToggleActorsTrigger class enable/disable other actors once the given amount of actors have entered the collision area */
UCLASS()
class COOPPLATFORMER_API AToggleActorsTrigger : public ATriggerBox
{
	GENERATED_BODY()
	
public:
	/** Default constructor for AToggleActorsTrigger*/
	AToggleActorsTrigger();

	/** The actor that will be enabled once the trigger happens */
	UPROPERTY(EditAnywhere, Category = "Customizable")
	TObjectPtr<AActor> EnableActor;

	/** The actor that will be disabled once the trigger happens */
	UPROPERTY(EditAnywhere, Category = "Customizable")
	TObjectPtr<AActor> DisableActor;

	/** The player actors in the map */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	TArray<TObjectPtr<AActor>> PlayerActors;

	/** The amount of actors that need to enter the collision area for things to trigger */
	UPROPERTY(EditAnywhere, Category = "Customizable")
	int TriggerAmount;

	/** Whether the event can trigger */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool CanTrigger;

	/** Fired when a character collides with the ToggleActorsTrigger - if requirements are satisfied it will enable/disable actors */
	UFUNCTION()
	void OnOverlapBegin(AActor* TriggerBoxActor, AActor* OtherActor);

	/** When players leave the collision area */
	UFUNCTION()
	void OnOverlapEnd(AActor* TriggerBoxActor, AActor* OtherActor);

	UFUNCTION(Reliable, NetMulticast)
	void MulticastDisableActors();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastEnableActors();

protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

private:

};
