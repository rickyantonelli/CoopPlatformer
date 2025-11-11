// Copyright Ricky Antonelli

#include "Character/MyPaperCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperSpriteComponent.h"
#include "PaperFlipbookComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

AMyPaperCharacter::AMyPaperCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UNovaCharacterMovementComponent>(AMyPaperCharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	bReplicates = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetWorldRotation(FRotator(0.0f, -90.0f, 0.0f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	Background = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Background"));
	Background->SetupAttachment(Camera);
	
	DoubleJumpFlipbook = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("DoubleJumpEffect"));
	DoubleJumpFlipbook->SetupAttachment(RootComponent);

	GetCharacterMovement()->bNotifyApex = true;
	BaseGravityScale = GetCharacterMovement()->GravityScale;

	// Misc movement
	IsHolding = false;
	MovementEnabled = true;
	bFirstPlayer = false;
	bPassingThrough = true;
	InitialFriction = 1.f;

	// Death
	DeathDuration = 1.0f;

	// Jumping
	WithinCoyoteTime = false;
	WithinDoubleJumpGrace = false;
	Jumping = false;
	DevInfiniteJump = false;
	HasJumpInput = true;
	CoyoteDuration = 0.5f;
	DevJumpResetTimer = 0.5f;
	JumpApexTimer = 0.2f;
	JumpApexGravityScale = 0.5f;
	DoubleJumpGrace = 0.2f;

	// Dash
	CanDash = false;
	DashSpeed = 2.0f;
	DashDuration = 1.0f;

	// Wall Jump
	bInWallJumpTimer = false;
	WallJumpGravityScale = 0.2f;
	WallJumpDuration = 0.5f;
	WallJumpGrace = 0.2f;
	WallJumpNudge = 150.f;
	WallJumpBounds = 100.0f;
	WallJumpMinVelocity = 20.0f;

	// Freeze
	bCanFreeze = false;
	bFrozen = false;
	FreezeVeloThreshold = 0.0f;
	FreezeFrictionCalc = 200.0f;

	ControlRotation = FRotator::ZeroRotator;
	PreviousVelocity = FVector::ZeroVector;
	PreFreezeVelocity = FVector::ZeroVector;
}

void AMyPaperCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	InitialFriction = GetCharacterMovement()->FallingLateralFriction;

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	SpriteComp = GetSprite();
	if (SpriteComp)
	{
		OriginalFlipbookScale = SpriteComp->GetRelativeScale3D();
	}

	if (Background)
	{
		BackgroundLoc = Background->GetComponentLocation().Y;
	}

}

void AMyPaperCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LockPosition != FVector::ZeroVector)
	{
		FVector SpringArmLocation = SpringArm->GetComponentLocation();
		if (bLockedX)
		{
			SpringArmLocation.X = LockPosition.X;
		}

		if (bLockedZ)
		{
			SpringArmLocation.Z = LockPosition.Z;
		}
		SpringArm->SetWorldLocation(SpringArmLocation);
	}

	PreviousVelocity = GetCharacterMovement()->Velocity;
	// if we ever get to a frame where we are jumping but also have 0 z velocity (vertical) then stop the jump
	// this stops us from sticking to the ceiling because we've hit the top but are holding jump
	if (Jumping)
	{
		if (GetCharacterMovement()->Velocity.Z == 0) StopJumping();
	}

	if (Background && Background->GetComponentLocation().X != BackgroundLoc)
	{
		Background->SetWorldLocation(FVector(Background->GetComponentLocation().X, BackgroundLoc , Background->GetComponentLocation().Z));
	}
}

void AMyPaperCharacter::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	// Coyote time application - allows the player a set amount of time to jump after leaving a platform
	EMovementMode NewMovementMode = GetCharacterMovement()->MovementMode;
	if (NewMovementMode == EMovementMode::MOVE_Falling)
	{
		WithinCoyoteTime = true;
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {WithinCoyoteTime = false; }, CoyoteDuration, false);
	}

}

void AMyPaperCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AMyPaperCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMyPaperCharacter::StopJumping);
		EnhancedInputComponent->BindAction(JumpReleaseAction, ETriggerEvent::Triggered, this, &AMyPaperCharacter::JumpReleased);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyPaperCharacter::Move);

		//Passing
		EnhancedInputComponent->BindAction(PassAction, ETriggerEvent::Triggered, this, &AMyPaperCharacter::Pass);

		//Extra Action
		EnhancedInputComponent->BindAction(ExtraAction, ETriggerEvent::Triggered, this, &AMyPaperCharacter::ExtraActionPressed);
		EnhancedInputComponent->BindAction(ExtraActionRelease, ETriggerEvent::Triggered, this, &AMyPaperCharacter::ExtraActionReleased);


		EnhancedInputComponent->BindAction(CountdownPingAction, ETriggerEvent::Triggered, this, &AMyPaperCharacter::CountdownPing);

	}

}

void AMyPaperCharacter::Move(const FInputActionValue& Value)
{
	if (MovementEnabled)
	{
		// input is a Vector2D
		FVector2D MovementVector = Value.Get<FVector2D>();

		LastMovementVector = MovementVector;

		const FVector ForwardDirection = GetActorForwardVector();
		const FVector RightDirection = GetActorUpVector();

		if (Controller != nullptr)
		{
			if (SpriteComp)
			{
				if (HasAuthority())
				{
					SpriteComp->SetRelativeScale3D(OriginalFlipbookScale * FVector(MovementVector.X >= 0 ? 1.f : -1.f, 1.f, 1.f));
				}
				else
				{
					// server rpc
					ServerFlipPlayer(MovementVector);
				}
			}
			if (bCanXMove) AddMovementInput(GetActorForwardVector(), MovementVector.X);
		}
	}
}


void AMyPaperCharacter::Pass(const FInputActionValue& Value)
{
	if (IsHolding && bPassingThrough && MovementEnabled && IsLocallyControlled())
	{
		ServerPassRPC();
	}
}

void AMyPaperCharacter::ServerPassRPC_Implementation()
{
	OnPassActivated.Broadcast();
}

void AMyPaperCharacter::ExtraActionPressed(const FInputActionValue& Value)
{
	// the game will support multiple types of token pickups
	// but will never have more than one active at a time

	// TODO: need to have these be server rpcs
	// still having disconnects with client not seeing what is happening on server
	// probably need to serverRPC->multicast for these actions

	if (!IsLocallyControlled()) return;

	if (CanDash)
	{
		Dash(Value);
		return;
	}

	if (bCanFreeze)
	{
		if (bInWallJumpTimer) return;
		if (bFrozen) return;
		if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking) return; // can only freeze in air

		PreFreezeVelocity = GetCharacterMovement()->Velocity;
		FreezeServerRPCFunction(PreFreezeVelocity);
	}
}

void AMyPaperCharacter::ExtraActionReleased(const FInputActionValue& Value)
{
	if (!IsLocallyControlled()) return;
	if (bFrozen)
	{
		UnFreezeServerRPCFunction(PreFreezeVelocity);
	}
}

void AMyPaperCharacter::Dash(const FInputActionValue& Value)
{
	if (IsLocallyControlled())
	{
		FVector DashDirection = FVector(LastMovementVector.X, 0.f, LastMovementVector.Y).GetSafeNormal();

		DashServerRPCFunction(DashDirection);
	}
}

void AMyPaperCharacter::FreezeServerRPCFunction_Implementation(FVector FreezeVelo)
{
	bCanXMove = false;
	bCanFreeze = false;
	bFrozen = true;
	MulticastFreezePlayer(FreezeVelo);
}

void AMyPaperCharacter::UnFreezeServerRPCFunction_Implementation(FVector FreezeVelo)
{
	UE_LOG(LogTemp, Warning, TEXT("Unfreeze Activated"));
	bCanXMove = true;
	bFrozen = false;
	MulticastUnfreezePlayer(FreezeVelo);

	// if absolute value of freezevelo x is greater than threshold, temporarily turn off lateral friction
	if (FMath::Abs(FreezeVelo.X) > FreezeVeloThreshold)
	{
		// calculate how long to turn off friction for based on current x velocity
		float FrictionTime = FMath::Clamp(FMath::Abs(FreezeVelo.X) / FreezeFrictionCalc, 0.1f, 1.5f);
		// print frictiontime and freezevelo x for debugging
		UE_LOG(LogTemp, Warning, TEXT("Friction Time: %f, FreezeVelo X: %f"), FrictionTime, FreezeVelo.X);
		MulticastApplyFriction(0, FrictionTime); // Apply zero friction for a short duration
	}
}

void AMyPaperCharacter::MulticastFreezePlayer_Implementation(FVector FreezeVelo)
{
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->GravityScale = 0.f;
}

void AMyPaperCharacter::MulticastUnfreezePlayer_Implementation(FVector FreezeVelo)
{
	GetCharacterMovement()->GravityScale = BaseGravityScale;
	GetCharacterMovement()->Velocity = FreezeVelo;
}

void AMyPaperCharacter::CountdownPing(const FInputActionValue& Value)
{
	if (IsLocallyControlled() && IsHolding)
	{
		ServerCountdownPingRPC();
	}
}

void AMyPaperCharacter::ServerCountdownPingRPC_Implementation()
{
	OnCountdownPingActivated.Broadcast();
}

void AMyPaperCharacter::DashServerRPCFunction_Implementation(FVector DashDir)
{
	MulticastApplyFriction(0, DashDuration); // Apply zero friction for the dash duration
	LaunchCharacter(DashDir * DashSpeed, true, true);
	CanDash = false;
}

void AMyPaperCharacter::ResetJumpAbility()
{
	if (HasAuthority())
	{
		// Resets the jump ability if the player is in the air
		// For catching the ball mid-air and getting a jump reset
		EMovementMode NewMovementMode = GetCharacterMovement()->MovementMode;

		if (NewMovementMode == EMovementMode::MOVE_Walking)
		{
			// This double jump grace will eliminate the bad feeling when you thought you did your first jump before catching
			WithinDoubleJumpGrace = true;

			FTimerHandle TimerHandler;
			GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {WithinDoubleJumpGrace = false; }, DoubleJumpGrace, false);
			return;
		} 

		// We want to allow the players a theoretical possibility to jump as much as they want
		// So increment by one, rather than just setting to 2
		JumpMaxCount += 1;

		// Need to think through if this is enough, or if we actually need to keep track of
		// something like JumpMaxCount - JumpCurrentCount >= 1
		if (DoubleJumpFlipbook)
		{
			DoubleJumpFlipbook->SetVisibility(true);
		}
	}
}

void AMyPaperCharacter::Landed(const FHitResult& Hit)
{
	// Once the player lands, reset anything gained while the player was in the air (jump reset)
	Super::Landed(Hit);
	// Dash prototype - holding off for now
	CanDash = false;
	if (HasAuthority()) JumpMaxCount = 1;
	GetCharacterMovement()->GravityScale = BaseGravityScale;
	Jumping = false;
	if (DoubleJumpFlipbook)
	{
		DoubleJumpFlipbook->SetVisibility(false);
	}
}

bool AMyPaperCharacter::CanJumpInternal_Implementation() const
{
	// overriding to see if we are within coyote time, if not just use Super
	if (WithinCoyoteTime && !Jumping)
	{
		return true;
	}

	else return Super::CanJumpInternal_Implementation();
}

void AMyPaperCharacter::Jump()
{
	// only allow if movement is enabled
	if (MovementEnabled && HasJumpInput)
	{
		Super::Jump();
	}
}

void AMyPaperCharacter::StopJumping()
{
	Super::StopJumping();
	// leaving this override in case we need it later
}

void AMyPaperCharacter::JumpReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("Jump Released"));
	// we care about this because we want the player to release the jump button before getting another jump
	HasJumpInput = true;
}

void AMyPaperCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	HasJumpInput = false;
	Jumping = true;

	if (bInWallJumpTimer)
	{
		OnWallExit(true);
	}

	if (DoubleJumpFlipbook)
	{
		DoubleJumpFlipbook->SetVisibility(false);
	}

	if (WithinDoubleJumpGrace)
	{
		JumpMaxCount += 1;
		WithinDoubleJumpGrace = false;
		if (DoubleJumpFlipbook)
		{
			DoubleJumpFlipbook->SetVisibility(true);
		}
	}

	if (DevInfiniteJump)
	{
		// For solo testing, just gives a large amount of jumps so you can test without passing
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {JumpMaxCount = 10; }, DevJumpResetTimer, false);
	}
}

void AMyPaperCharacter::OnDeath()
{
	if (IsLocallyControlled())
	{
		// On death we instantly return the player to spawn
		// but we want to disable controls for a short amount of time so player's dont instantly move on respawn
		// set it up with MovementEnabled so that players still have things like pause menu still available
		MovementEnabled = false;
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {MovementEnabled = true; }, DeathDuration, false);
	}
	if (SpriteComp && HasAuthority())
	{
		bDead = true;
		SpriteComp->SetVisibility(false);
		FTimerHandle TimerHandler2;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler2, [&]() {SpriteComp->SetVisibility(true); bDead = false; }, DeathDuration, false);
	}
}

void AMyPaperCharacter::NotifyJumpApex()
{
	GravityAtApex();
	Super::NotifyJumpApex();

}

void AMyPaperCharacter::GravityAtApex() const
{
	// in order to get a smoother controller, we slightly reduce the gravity scale of the player when they are at a jump apex
	// for a short amount of time - gives a floating feeling at the top
	

	// TODO: Reimplement this - it was breaking freeze token
	//if (Jumping && !bFrozen)
	//{
	//	GetCharacterMovement()->GravityScale = BaseGravityScale * JumpApexGravityScale;
	//	FTimerHandle TimerHandler;
	//	GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {GetCharacterMovement()->GravityScale = BaseGravityScale; }, JumpApexTimer, false);
	//	
	//}
	//GetCharacterMovement()->bNotifyApex = true;
}

void AMyPaperCharacter::RemoveBallArrivingWidget()
{
	// TODO: Can open this up by allowing a user widget to be passed in
	if (IsLocallyControlled() && BallArrivingWidget && IsValid(BallArrivingWidget) && BallArrivingWidget->IsInViewport())
	{
		BallArrivingWidget->RemoveFromParent();
	}
}

void AMyPaperCharacter::ApplyDashToken()
{
	// ensure that we are in the air
	EMovementMode PlayerMovementMode = GetCharacterMovement()->MovementMode;

	if (PlayerMovementMode == EMovementMode::MOVE_Walking) return;

	CanDash = true;
}


void AMyPaperCharacter::ApplyFreezeToken()
{
	if (HasAuthority())
	{
		bCanFreeze = true;
	}
}

void AMyPaperCharacter::MulticastApplyFriction_Implementation(int Friction, float FrictionTimer)
{
	GetCharacterMovement()->FallingLateralFriction = Friction;

	FTimerHandle TimerHandler;
	GetWorld()->GetTimerManager().SetTimer(TimerHandler, [this]() {GetCharacterMovement()->FallingLateralFriction = InitialFriction; }, FrictionTimer, false);
}

void AMyPaperCharacter::ServerFlipPlayer_Implementation(FVector2D MovementVector)
{
	SpriteComp->SetRelativeScale3D(OriginalFlipbookScale * FVector(MovementVector.X >= 0 ? 1.f : -1.f, 1.f, 1.f));
}

void AMyPaperCharacter::MulticastPauseGame_Implementation(UUserWidget* myWidget)
{
	// UGameplayStatics::SetGamePaused(GetWorld(), true);
	checkf(PauseMenuOverlayWidgetClass, TEXT("Pause Menu Overlay Widget class uninitialized"));
	PauseMenuWidget = CreateWidget<UUserWidget>(GetWorld(), PauseMenuOverlayWidgetClass);
	PauseMenuWidget->AddToViewport();
	APlayerController* myController = GetWorld()->GetFirstPlayerController();
	if (myController)
	{
		myController->SetPause(true);
		myController->bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		myController->SetInputMode(InputMode);
	}

}

void AMyPaperCharacter::MulticastResumeGame_Implementation(UUserWidget* myWidget=nullptr)
{
	APlayerController* myController = GetWorld()->GetFirstPlayerController();
	if (myController)
	{
		myController->bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		myController->SetInputMode(InputMode);
		myController->SetPause(false);
		if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
		{
			PauseMenuWidget->RemoveFromParent();
		}
	}
	// get rid of menus, then unpause
	UGameplayStatics::SetGamePaused(GetWorld(), false);

}

void AMyPaperCharacter::BallArrivingClientRPCFunction_Implementation()
{
	// RPC to display the Ball Arriving widget when a ball is on its way to the player
	checkf(BallArrivingOverlayWidgetClass, TEXT("Ball Arriving Overlay Widget class uninitialized"));
	BallArrivingWidget = CreateWidget<UUserWidget>(GetWorld(), BallArrivingOverlayWidgetClass);

	if (BallArrivingWidget) BallArrivingWidget->AddToViewport();
}

void AMyPaperCharacter::RemoveBallArrivingClientRPCFunction_Implementation()
{
	RemoveBallArrivingWidget();
}

void AMyPaperCharacter::CountdownPingClientRPCFunction_Implementation()
{
	// RPC to display the Ball Arriving widget when a ball is on its way to the player
	checkf(CountdownPingOverlayWidgetClass, TEXT("Ball Arriving Overlay Widget class uninitialized"));
	CountdownWidget = CreateWidget<UUserWidget>(GetWorld(), CountdownPingOverlayWidgetClass);

	if (CountdownWidget) CountdownWidget->AddToViewport();
}

void AMyPaperCharacter::OnRep_IsHolding()
{
	RemoveBallArrivingWidget();
}

void AMyPaperCharacter::OnWallHit(bool bLeft)
{
	EMovementMode NewMovementMode = GetCharacterMovement()->MovementMode;

	if (NewMovementMode == EMovementMode::MOVE_Walking) return;
	if (bInWallJumpTimer) return;

	bInWallJumpTimer = true;

	FTimerHandle TimerHandler;
	GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {if (bInWallJumpTimer) OnWallExit(); }, WallJumpDuration, false);

	GetCharacterMovement()->GravityScale = WallJumpGravityScale;
	GetCharacterMovement()->Velocity = FVector(0.f, 0.f, 0.f);
	
	if (HasAuthority()) bCanXMove = false;
	JumpMaxCount += 1;
	LastWallHitLeft = bLeft;
}

void AMyPaperCharacter::OnWallExit(bool FromJump)
{
	bInWallJumpTimer = false;
	GetCharacterMovement()->GravityScale = BaseGravityScale;
	if (HasAuthority()) bCanXMove = true;

	if (FromJump)
	{
		//if (!IsLocallyControlled()) return;


		FVector KickOffVelocity;
		float KickOffHorizontal = WallJumpNudge; 
		KickOffVelocity.X = LastWallHitLeft ? KickOffHorizontal : -KickOffHorizontal;
		KickOffVelocity.Z = 0.f; // just lateral

		LaunchCharacter(KickOffVelocity, true, false);

		return;
	}

	FTimerHandle TimerHandler;
	GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {if (JumpMaxCount > 1) JumpMaxCount -= 1; }, WallJumpGrace, false);
}



void AMyPaperCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPaperCharacter, IsHolding);
	DOREPLIFETIME(AMyPaperCharacter, CanDash);
	DOREPLIFETIME(AMyPaperCharacter, bCanFreeze);
	DOREPLIFETIME(AMyPaperCharacter, bCanXMove);
	DOREPLIFETIME(AMyPaperCharacter, bFrozen);
	DOREPLIFETIME(AMyPaperCharacter, bPassingThrough);
	DOREPLIFETIME(AMyPaperCharacter, JumpMaxCount);
	DOREPLIFETIME(AMyPaperCharacter, bFirstPlayer);
	DOREPLIFETIME(AMyPaperCharacter, ControlRotation);
}

