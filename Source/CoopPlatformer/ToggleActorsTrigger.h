// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/Actor.h"
#include "ToggleActorsTrigger.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API AToggleActorsTrigger : public ATriggerBox
{
	GENERATED_BODY()
	
public:
	AToggleActorsTrigger();

	UPROPERTY(EditAnywhere, Category = "Customizable")
	AActor* EnableActor;

	UPROPERTY(EditAnywhere, Category = "Customizable")
	AActor* DisableActor;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	TArray<AActor*> PlayerActors;

	UPROPERTY(EditAnywhere, Category = "Customizable")
	int TriggerAmount;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool CanTrigger;

	UFUNCTION()
	void OnOverlapBegin(AActor* TriggerBoxActor, AActor* OtherActor);

	UFUNCTION()
	void OnOverlapEnd(AActor* TriggerBoxActor, AActor* OtherActor);

protected:
	virtual void BeginPlay() override;

private:

};
