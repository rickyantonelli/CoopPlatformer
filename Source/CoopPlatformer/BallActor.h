// Copyright Ricky Antonelli

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
	ABallActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	USceneComponent* RootComp;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UCapsuleComponent* Sphere;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BallMovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	float PassCooldownDuration;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool NoPassCooldown;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool IsMoving;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsAttached, Category = "Debug")
	bool IsAttached;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool CanPass;

	UFUNCTION(BlueprintCallable)
	void BeginPassCooldown();

	UFUNCTION()
	void OnRep_IsAttached();
};
