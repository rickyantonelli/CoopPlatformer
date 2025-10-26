// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "TeleporterActor.generated.h"

UCLASS()
class COOPPLATFORMER_API ATeleporterActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATeleporterActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> TPMesh1;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> TPMesh2;

	/** When the player collides, disables or enables the PressurePlated Actor */
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	TArray<AActor*> TPActorsOnCD;

	UPROPERTY(EditAnywhere, Category = "Custom")
	float TeleportCooldown;

	/** How much camera lag should be offset when teleporting, for a smoother feel */
	UPROPERTY(EditAnywhere, Category = "Custom")
	float CameraLagOffset;

	UPROPERTY(EditAnywhere, Category = "Custom")
	float CameraLagTime;


};
