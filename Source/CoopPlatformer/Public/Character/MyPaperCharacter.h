// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "PaperCharacter.h"
#include "Character/NovaCharacterMovementComponent.h"
#include "MyPaperCharacter.generated.h"

// TODO: currently no reason for this to need to be dynamic
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBallPassActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSwapActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCountdownPingActivated);
/**
 * The PaperCharacter class is the player class for the game - holds a variety of responsibilities
 * Player Movement - moving left and right
 * Jumping - holding for longer jump, resetting jump in mid air
 * Passing - firing delegate to pass the ball
 * Widgets - notifying player when ball is arriving
 */
UCLASS()
class COOPPLATFORMER_API AMyPaperCharacter : public APaperCharacter
{
	GENERATED_BODY()

public:
	/** Default constructor for AMyPaperCharacter - Sets components, replication, and initializes variables*/
	AMyPaperCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	/** Override for BeginPlay*/
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for passing input */
	void Pass(const FInputActionValue& Value);

	/** Called for dashing input */
	void Dash(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void FreezeServerRPCFunction(FVector FreezeVelo);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void UnFreezeServerRPCFunction(FVector FreezeVelo);

	/** Called for dashing input */
	void ExtraActionPressed(const FInputActionValue& Value);

	void ExtraActionReleased(const FInputActionValue& Value);

	/** Called for countdown ping input */
	void CountdownPing(const FInputActionValue& Value);

public:
	/** Override for Tick*/
	virtual void Tick(float DeltaTime) override;

	/** Override for SetupPlayerInputComponent to bind actions*/
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Overriding the landed event to reset jump ability back to default and gravity scale */
	virtual void Landed(const FHitResult& Hit) override;

	/** Overriding CanJump implementation to check for coyote time before applying */
	virtual bool CanJumpInternal_Implementation() const override;

	/** Overriding OnJumped implementation to apply variables and implement dev cheats when needed */
	virtual void OnJumped_Implementation() override;

	/** Overriding Jump to make sure that internal jumping checks are applied */
	virtual void Jump() override;

	/** Overrides StopJumping */
	virtual void StopJumping() override;

	/** Overrides NotifyJumpApex so that player gravity scale can be lowered at apex - gives a floaty feeling to player */
	virtual void NotifyJumpApex() override;

	/** Overrides OnMovementModeChanged to handle coyote time implementation */
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	/** Required for replicated variables */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Client RPC to display a widget notifying the player of a ball's arrival */
	UFUNCTION(Client, Unreliable, BlueprintCallable)
	void BallArrivingClientRPCFunction();

	UFUNCTION(Client, Unreliable, BlueprintCallable)
	void CountdownPingClientRPCFunction();

	/** The player's camera spring arm*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	/** The capsule component for the player to handle collision*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Capsule, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> Capsule;

	/** The camera component for the player*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperFlipbookComponent> SpriteComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperFlipbookComponent> DoubleJumpFlipbook;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperSpriteComponent> Background;

	/** The default mapping context for the player */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** The action to bind to jump */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;
	
	/** The action to bind to when jump is released - for allowing flexibility in jump height */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpReleaseAction;

	/** The action to bind to lateral movement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/** The action to bind to passing of the ball */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PassAction;

	/** The action to bind to player dashing*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ExtraAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ExtraActionRelease;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CountdownPingAction;

	/** The class of the widget that notifies the player of the ball's arrival */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> BallArrivingOverlayWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> CountdownPingOverlayWidgetClass;

	/** The class of the widget that notifies the player of the ball's arrival */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuOverlayWidgetClass;

	/*
	==================================================
					CUSTOMIZABLE VALUES
	==================================================
	*/

	/** How long a player should have their movement turned off for after death */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Death")
	float DeathDuration;

	/** How long coyote time should last - meaning how much error time a player has to still jump after leaving a platform */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Jump")
	float CoyoteDuration;

	UPROPERTY(EditAnywhere, Category = "Customizable Values|Jump")
	float DoubleJumpGrace;

	/** For testing - allows the dev to infinitely jump instead of passing back and forth to get jump resets */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Cheats")
	bool DevInfiniteJump;

	/** How frequently the dev can infinite jump */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Cheats")
	float DevJumpResetTimer;

	/** How long the gravity is reduced at apex on a jump */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Jump")
	float JumpApexTimer;

	/** The scale of gravity reduction when a player is at apex */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Jump")
	float JumpApexGravityScale;

	/** The time that the dash should occur for */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Dash")
	float DashDuration;

	/** The speed of the dash */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Dash")
	float DashSpeed;

	/** Threshold velo for when to step in and turn off lateral air friction */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Freeze")
	float FreezeVeloThreshold;

	/** The amount you divide the x velo to calculate how long to turn off friction for */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Freeze")
	float FreezeFrictionCalc;

	/** The scale of gravity reduction when attaching to wall for wall jump */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Wall Jump")
	float WallJumpGravityScale;

	/** The length of time to hang for a wall jump */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Wall Jump")
	float WallJumpDuration;

	/** Grace period to allow for a jump after leaving contact with the wall */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Wall Jump")
	float WallJumpGrace;

	/** Slight nudge when jumping off a wall to propel in direction away from wall */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Wall Jump")
	float WallJumpNudge;

	/** Upper and lower bounds to be eligible for a wall jump */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Wall Jump")
	float WallJumpBounds;

	/** The minimum incoming X velocity to allow for wall jumps */
	UPROPERTY(EditAnywhere, Category = "Customizable Values|Wall Jump")
	float WallJumpMinVelocity;

	/** Whether the player is holding the ball */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsHolding, BlueprintReadWrite, Category = "Debug")
	bool IsHolding;

	/** Whether movement is enabled - for handling a small break after death. This allows players to still do things like opening menu while dead */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Debug")
	bool MovementEnabled;

	/** Whether a player is within coyote time and can still jump despite not being grounded */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool WithinCoyoteTime;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool WithinDoubleJumpGrace;

	/** Whether a player is in the act of jumping */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool Jumping;

	/** Whether the player has an input available to jump. This is so that the player has to press jump every time they want to, not hold it down */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool HasJumpInput;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool bInWallJumpTimer;

	/** Whether the player can dash*/
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool CanDash;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool bPassingThrough;

	/** Whether the player can dash*/
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool bCanFreeze;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool bFrozen;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadOnly, Category = "Debug")
	bool bFirstPlayer;

	bool bDead = false;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool bCanXMove = true;

	bool LastWallHitLeft = true;

	UPROPERTY(VisibleAnywhere, Replicated)
	FRotator ControlRotation;

	FVector2D LastMovementVector;

	FVector PreviousVelocity;

	FVector PreFreezeVelocity;

	/** The normal gravity scale, so that we can revert back to this after getting out of an apex */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	float BaseGravityScale;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	float InitialFriction;

	/** The player's original spawn location - so that if they die without collecting a checkpoint they come back to here */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	FVector OriginalFlipbookScale;

	/** Delegate for the ball being passed - which is picked up by the player controlller and called on the server */
	UPROPERTY(BlueprintAssignable)
	FBallPassActivated OnPassActivated;

	UPROPERTY(BlueprintAssignable)
	FSwapActivated OnSwapActivated;

	UPROPERTY(BlueprintAssignable)
	FCountdownPingActivated OnCountdownPingActivated;

	/** The the widget that notifies the player of the ball's arrival */
	TObjectPtr<UUserWidget> BallArrivingWidget;

	TObjectPtr<UUserWidget> CountdownWidget;

	/** The the widget that notifies the player of the ball's arrival */
	TObjectPtr<UUserWidget> PauseMenuWidget;

	/** When server changes IsHolding */
	UFUNCTION()
	void OnRep_IsHolding();

	/** Resets the jump ability of a player - a core mechanic that allows for double jumping if ball is caught in mid air */
	UFUNCTION()
	void ResetJumpAbility();

	/** Event when a player dies, which returns them back to a checkpoint or spawn and temporarily disables movement */
	UFUNCTION()
	void OnDeath();

	/** Event for jump button being released - ends a jump early and allows flexibility in jump height */
	UFUNCTION()
	void JumpReleased();

	/** Called to apply the gravity scale change when a player is at the apex of their jump */
	UFUNCTION()
	void GravityAtApex() const;

	/** Removes the ball arriving widget once the player has caught the ball */
	UFUNCTION()
	void RemoveBallArrivingWidget();

	UFUNCTION()
	void ApplyDashToken();

	UFUNCTION()
	void ApplyFreezeToken();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void MulticastPauseGame(UUserWidget* myWidget);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void MulticastResumeGame(UUserWidget* myWidget);

	UFUNCTION(Server, Reliable)
	void ServerFlipPlayer(FVector2D MovementVector);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void DashServerRPCFunction(FVector DashDir);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void MulticastApplyFriction(int Friction, float FrictionTimer);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void MulticastFreezePlayer(FVector FreezeVelo);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void MulticastUnfreezePlayer(FVector FreezeVelo);

	UFUNCTION()
	void OnWallHit(bool bLeft);

	UFUNCTION()
	void OnWallExit(bool FromJump=false);

};
