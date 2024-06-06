// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/BoxComponent.h"
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
	AMyPaperCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for passing input */
	void Pass(const FInputActionValue& Value);

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult& Hit) override;

	virtual bool CanJumpInternal_Implementation() const override;

	virtual void OnJumped_Implementation() override;

	virtual void Jump() override;

	virtual void StopJumping() override;

	virtual void NotifyJumpApex() override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void BallArrivingClientRPCFunction();

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
	UInputAction* JumpReleaseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PassAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* BallHolder;

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	float DeathDuration;

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	float CoyoteDuration;

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	bool DevInfiniteJump;

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	float DevJumpResetTimer;

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	float JumpApexTimer;

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	float JumpApexGravityScale;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadWrite, Category = "Debug")
	bool IsHolding;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Debug")
	bool MovementEnabled;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool WithinCoyoteTime;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool Jumping;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool HasJumpInput;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	float BaseGravityScale;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	FVector SpawnLocation;

	UPROPERTY(BlueprintAssignable)
	FBallPassActivated OnPassActivated;

	FRotator ControlRotation;

	UFUNCTION()
	void ResetJumpAbility();

	UFUNCTION()
	void OnDeath();

	UFUNCTION()
	void JumpReleased();

	UFUNCTION()
	void GravityAtApex() const;

};
