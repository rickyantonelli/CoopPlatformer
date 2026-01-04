// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Controller/Controller2D.h"
#include "PaperFlipbook.h"
#include "PaperFlipBookComponent.h"
#include "BallReboundComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPPLATFORMER_API UBallReboundComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBallReboundComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool bControllersBound = false;

	AController2D* AuthController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TSubclassOf<AActor> SparkActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	float SparkLifetime = 1.0f;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnSparkEffect(FVector SpawnLocation);
};
