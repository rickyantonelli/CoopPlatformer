// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "CharacterMovementComponent.generated.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "PaperCharacter.h"
#include "MyPaperCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBallPassActivated);

/**
 *
 */
UCLASS()
class COOPPLATFORMER_API AMyPaperCharacter : public APaperCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyPaperCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for passing input */
	void Pass(const FInputActionValue& Value);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult& Hit) override;

	virtual bool CanJumpInternal_Implementation() const override;

	virtual void OnJumped_Implementation() override;

	virtual void Jump() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Capsule, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* Capsule;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PassAction;

	UPROPERTY(VisibleAnywhere)
	UCharacterMovementComponent* MyCharacterMovement;

	UPROPERTY(EditAnywhere)
	float DeathDuration;

	UPROPERTY(VisibleAnywhere)
	bool IsHolding;

	UPROPERTY(VisibleAnywhere)
	bool CanJumpReset;

	UPROPERTY(VisibleAnywhere)
	bool MovementEnabled;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(BlueprintAssignable)
	FBallPassActivated OnPassActivated;

	UFUNCTION()
	void ResetJumpAbility();

	UFUNCTION()
	void OnDeath();

};
