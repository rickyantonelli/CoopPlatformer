// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "ActorTriggerArea.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API AActorTriggerArea : public ATriggerBox
{
	GENERATED_BODY()
	
	AActorTriggerArea();

protected:

	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Category = "Customizable")
	TObjectPtr<AActor> EnableActor;

	UPROPERTY(EditAnywhere, Category = "Customizable")
	TObjectPtr<AActor> DisableActor;

	UPROPERTY(EditAnywhere, Category = "Customizable")
	int TriggerAmount;

	UPROPERTY(EditAnywhere, Category = "Customizable")
	bool EnableChoice;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	TArray<TObjectPtr<AActor>> PlayerActors;

	UFUNCTION()
	void OnOverlapBegin(AActor* TriggerBoxActor, AActor* OtherActor);

	/** When players leave the collision area */
	UFUNCTION()
	void OnOverlapEnd(AActor* TriggerBoxActor, AActor* OtherActor);

	UFUNCTION(Reliable, NetMulticast)
	void MulticastDisableActor();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastEnableActor();
};
