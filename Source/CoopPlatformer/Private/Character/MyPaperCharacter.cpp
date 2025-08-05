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
	
	DoubleJumpFlipbook = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("DoubleJumpEffect"));
	DoubleJumpFlipbook->SetupAttachment(RootComponent);

	GetCharacterMovement()->bNotifyApex = true;

	IsHolding = false;
	MovementEnabled = true;
	WithinCoyoteTime = false;
	WithinDoubleJumpGrace = false;
	Jumping = false;
	DevInfiniteJump = false;
	HasJumpInput = true;
	bFirstPlayer = false;

	// Dash prototype - holding off for now
	CanDash = false;

	BaseGravityScale = GetCharacterMovement()->GravityScale;

	DeathDuration = 1.0f;
	CoyoteDuration = 0.5f;
	DevJumpResetTimer = 0.5f;
	JumpApexTimer = 0.2f;
	JumpApexGravityScale = 0.5f;
	DashSpeed = 2.0f;
	DashDuration = 1.0f;
	DoubleJumpGrace = 0.2f;

	ControlRotation = FRotator::ZeroRotator;
}

void AMyPaperCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();

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

}

void AMyPaperCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	// if we ever get to a frame where we are jumping but also have 0 z velocity (vertical) then stop the jump
	// this stops us from sticking to the ceiling because we've hit the top but are holding jump
	if (Jumping)
	{
		if (GetCharacterMovement()->Velocity.Z == 0) StopJumping();
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

		//Dashing
		// Dash prototype - holding off for now
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &AMyPaperCharacter::Dash);

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
			AddMovementInput(GetActorForwardVector(), MovementVector.X);
		}
	}
}


void AMyPaperCharacter::Pass(const FInputActionValue& Value)
{
	if (IsHolding && MovementEnabled && IsLocallyControlled())
	{
		OnPassActivated.Broadcast();
	}
}

void AMyPaperCharacter::Dash(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("DASH"));
	// Dash prototype - holding off for now
	if (!CanDash) return;


	if (IsLocallyControlled())
	{
		FVector DashDirection = FVector(LastMovementVector.X, 0.f, LastMovementVector.Y).GetSafeNormal();

		DashServerRPCFunction(DashDirection);
	}
}

void AMyPaperCharacter::DashServerRPCFunction_Implementation(FVector DashDir)
{
	MulticastApplyFriction(0); // Apply zero friction for the dash duration
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
		UE_LOG(LogTemp, Log, TEXT("Coyote jumping"));
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
	// we care about this because we want the player to release the jump button before getting another jump
	HasJumpInput = true;
}

void AMyPaperCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	HasJumpInput = false;
	Jumping = true;

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
	// GetCharacterMovement()->GravityScale = BaseGravityScale;
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
		SpriteComp->SetVisibility(false);
		FTimerHandle TimerHandler2;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler2, [&]() {SpriteComp->SetVisibility(true);}, DeathDuration, false);
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
	if (Jumping)
	{
		GetCharacterMovement()->GravityScale = BaseGravityScale * JumpApexGravityScale;
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {GetCharacterMovement()->GravityScale = BaseGravityScale; }, JumpApexTimer, false);
		
	}
	GetCharacterMovement()->bNotifyApex = true;
}

void AMyPaperCharacter::RemoveBallArrivingWidget()
{
	// TODO: Can open this up by allowing a user widget to be passed in
	if (BallArrivingWidget && BallArrivingWidget->IsInViewport() && IsHolding)
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

void AMyPaperCharacter::MulticastApplyFriction_Implementation(int Friction)
{
	int InitialFriction = GetCharacterMovement()->FallingLateralFriction;
	GetCharacterMovement()->FallingLateralFriction = Friction;

	FTimerHandle TimerHandler;
	GetWorld()->GetTimerManager().SetTimer(TimerHandler, [this, InitialFriction]() {GetCharacterMovement()->FallingLateralFriction = InitialFriction; }, DashDuration, false);
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
	else
	{
		UE_LOG(LogTemp, Log, TEXT("NO CONTROLLER!!!"))
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

void AMyPaperCharacter::OnRep_IsHolding()
{
	RemoveBallArrivingWidget();
}

void AMyPaperCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPaperCharacter, IsHolding);
	DOREPLIFETIME(AMyPaperCharacter, CanDash);
	DOREPLIFETIME(AMyPaperCharacter, JumpMaxCount);
	DOREPLIFETIME(AMyPaperCharacter, bFirstPlayer);
	DOREPLIFETIME(AMyPaperCharacter, ControlRotation);
}
