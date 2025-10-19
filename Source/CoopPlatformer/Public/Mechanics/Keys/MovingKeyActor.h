// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include <Components/BoxComponent.h>
#include <PaperSpriteComponent.h>
#include "Components/ArrowComponent.h"
#include "Components/SplineComponent.h"
#include "Character/MyPaperCharacter.h"
#include "GameFramework/Actor.h"
#include "MovingKeyActor.generated.h"

UCLASS()
class COOPPLATFORMER_API AMovingKeyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMovingKeyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UPaperSpriteComponent> Sprite;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	USplineComponent* Spline;

	UPROPERTY(EditAnywhere, Replicated)
	FVector StartPoint;

	/** The point that the actor should move towards */
	UPROPERTY(EditAnywhere, Replicated)
	FVector EndPoint;

	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, Category = "Customizable")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, Category = "Customizable")
	float CooldownTimer;

	UPROPERTY(VisibleAnywhere, Replicated)
	bool bActivated;

	UPROPERTY(VisibleAnywhere, Replicated)
	bool bOnCooldown;

	UPROPERTY(VisibleAnywhere, Replicated)
	bool bMovingForward;

	UPROPERTY(VisibleAnywhere, Replicated)
	float SplineDistance;

};
