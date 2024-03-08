// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "MyPaperCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/InputComponent.h"

#include "BallActor.generated.h"


UCLASS()
class COOPPLATFORMER_API ABallActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABallActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//void Pass(const FInputActionValue& Value);

	// void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifeTimeProps) const override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BallMovementSpeed;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	USceneComponent* RootComp;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UCapsuleComponent* Sphere;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere)
	float PassCooldownDuration;

	UPROPERTY(VisibleAnywhere)
	bool NoPassCooldown;

	UPROPERTY(VisibleAnywhere)
	bool IsMoving;

	UPROPERTY(VisibleAnywhere)
	bool IsAttached;

	UPROPERTY(VisibleAnywhere)
	bool IsHeld; // simple bool to see if the ball is being held by anyone yet

	UPROPERTY(VisibleAnywhere)
	bool CanPass;

	UFUNCTION()
	void BeginPassCooldown();
};
