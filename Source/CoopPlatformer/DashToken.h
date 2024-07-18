// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DashToken.generated.h"

UCLASS()
class COOPPLATFORMER_API ADashToken : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADashToken();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void CollectDash();

	UFUNCTION()
	void SetCollectable();

	/** The root component of the pressure plate actor */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	USceneComponent* RootComp;

	/** The pressure plate's static mesh*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Customizable")
	float RespawnTimer;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool CanBeCollected;

};
