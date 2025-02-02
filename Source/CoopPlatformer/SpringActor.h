// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "MyPaperCharacter.h"
#include "GameFramework/Actor.h"
#include "SpringActor.generated.h"

UCLASS()
class COOPPLATFORMER_API ASpringActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpringActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	float LaunchPower;

	/** The root component of the pressure plate actor */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	/** The pressure plate's static mesh*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> BoxMesh;

	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
