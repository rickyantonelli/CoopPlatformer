// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Controller/Controller2D.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "PassSwapActor.generated.h"

UCLASS()
class COOPPLATFORMER_API APassSwapActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APassSwapActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> FirstSetActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> SecondSetActors;

	TObjectPtr<ATriggerBox> ActivatedArea;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSwapActors();

	UFUNCTION()
	void OnPassActivated();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) //, Replicated)
	bool FirstSetActive = true;

	bool BindingsSet = false;

	bool Activated = true;

};
