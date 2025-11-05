// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Controller/Controller2D.h"
#include "GameFramework/Actor.h"
#include "Systems/MyGameStateBase.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customizable")
	TArray<AActor*> FirstSetActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customizable")
	TArray<AActor*> SecondSetActors;

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	TObjectPtr<AActor> ActivationArea;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSwapActors();

	UFUNCTION()
	void OnPassActivated();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) //, Replicated)
	bool FirstSetActive = true;

	bool BindingsSet = false;

	UPROPERTY(Replicated)
	bool bActivated = false;

	UPROPERTY(Replicated)
	TArray<AActor*> CurrentActiveActors;

	UFUNCTION()
	virtual void OnActivateTriggerBeginOverlap(AActor* PlayerActor, AActor* OtherActor);

	UFUNCTION()
	virtual void OnActivateTriggerEndOverlap(AActor* PlayerActor, AActor* OtherActor);

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	TObjectPtr<UPaperSprite> OffSprite;

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	TObjectPtr<UPaperSprite> OnSprite;


};
