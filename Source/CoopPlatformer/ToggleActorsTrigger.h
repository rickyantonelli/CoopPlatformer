// Fill out your copyright notice in the Description page of Project Settings.

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
	UPROPERTY(EditAnywhere)
	AActor* SingleActor;

	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> ActivePlayers;

	UFUNCTION()
	void OnOverlapBegin(AActor* TriggerBoxActor, AActor* OtherActor);

	UFUNCTION()
	void OnOverlapEnd(AActor* TriggerBoxActor, AActor* OtherActor);

protected:
	virtual void BeginPlay() override;

private:

};
