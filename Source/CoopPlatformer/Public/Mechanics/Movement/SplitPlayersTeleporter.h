// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "SplitPlayersTeleporter.generated.h"

UCLASS()
class COOPPLATFORMER_API ASplitPlayersTeleporter : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASplitPlayersTeleporter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> Teleporter;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> P1Teleporter;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> P2Teleporter;

	UFUNCTION()
	void OnTeleportDistribute(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTeleportReturn(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void ApplyCameraLag(AActor* PlayerActor);

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool CanTeleport;

	UPROPERTY(EditAnywhere, Category = "Custom")
	float TeleportCooldown;

	/** How much camera lag should be offset when teleporting, for a smoother feel */
	UPROPERTY(EditAnywhere, Category = "Custom")
	float CameraLagOffset;

	UPROPERTY(EditAnywhere, Category = "Custom")
	float CameraLagTime;

	bool bP1Taken = false;
};
