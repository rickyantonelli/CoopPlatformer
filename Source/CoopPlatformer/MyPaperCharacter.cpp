// Copyright Ricky Antonelli

#include "MyPaperCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

AMyPaperCharacter::AMyPaperCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	// favor a box component to hold over the player's head
	BallHolder = CreateDefaultSubobject<UBoxComponent>(TEXT("BallHolder"));
	BallHolder->SetupAttachment(RootComponent);
	BallHolder->SetCollisionProfileName(FName("OverlapAllDynamic"));
	BallHolder->SetIsReplicated(true);

	GetCharacterMovement()->bNotifyApex = true;

	IsHolding = false;
	MovementEnabled = true;
	WithinCoyoteTime = false;
	Jumping = false;
	DevInfiniteJump = false;
	HasJumpInput = true;

	BaseGravityScale = GetCharacterMovement()->GravityScale;

	DeathDuration = 1.0f;
	CoyoteDuration = 0.5f;
	DevJumpResetTimer = 0.5f;
	JumpApexTimer = 0.2f;
	JumpApexGravityScale = 0.5f;

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

}

void AMyPaperCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// debugging for a persistent bug we've been encountering
	// this will hang around for a bit, just to ensure the bug is fixed
	if (!IsHolding)
	{
		TArray<AActor*> ChildActors;
		GetAllChildActors(ChildActors);
		for (AActor* ChildActor : ChildActors)
		{
			if (ChildActor->ActorHasTag("Ball"))
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "It broke here");
			}
		}
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

	}

}

void AMyPaperCharacter::Move(const FInputActionValue& Value)
{
	if (MovementEnabled)
	{
		// input is a Vector2D
		FVector MovementVector = Value.Get<FVector>();

		if (Controller != nullptr)
		{
			// Standard left and right movement
			if (MovementVector.X > 0)
			{
				ControlRotation.Yaw = 0;
				GetController()->SetControlRotation(ControlRotation);
				AddMovementInput(GetActorForwardVector(), MovementVector.X);
			}
			else
			{
				ControlRotation.Yaw = 180;
				GetController()->SetControlRotation(ControlRotation);
				AddMovementInput(GetActorForwardVector(), -MovementVector.X);
			}
		}
	}
}


void AMyPaperCharacter::Pass(const FInputActionValue& Value)
{
	if (IsHolding && MovementEnabled && IsLocallyControlled())
	{
		OnPassActivated.Broadcast(this);
	}
}

void AMyPaperCharacter::ResetJumpAbility()
{
	// Resets the jump ability if the player is in the air
	// For catching the ball mid-air and getting a jump reset
	EMovementMode NewMovementMode = GetCharacterMovement()->MovementMode;
	//IsHolding = true;

	if (NewMovementMode == EMovementMode::MOVE_Walking) return;
	JumpMaxCount = 2;
}

void AMyPaperCharacter::Landed(const FHitResult& Hit)
{
	// Once the player lands, reset anything gained while the player was in the air (jump reset)
	Super::Landed(Hit);
	JumpMaxCount = 1;
	GetCharacterMovement()->GravityScale = BaseGravityScale;
	Jumping = false;
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
	// we care about this because we want the player to release the jump button before getting another jump
	HasJumpInput = true;
}

void AMyPaperCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	HasJumpInput = false;
	Jumping = true;
	GetCharacterMovement()->GravityScale = BaseGravityScale;
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

void AMyPaperCharacter::BallArrivingClientRPCFunction_Implementation()
{
	// RPC to display the Ball Arriving widget when a ball is on its way to the player
	checkf(BallArrivingOverlayWidgetClass, TEXT("Ball Arriving Overlay Widget class uninitialized"));
	BallArrivingWidget = CreateWidget<UUserWidget>(GetWorld(), BallArrivingOverlayWidgetClass);

	BallArrivingWidget->AddToViewport();
}

void AMyPaperCharacter::OnRep_IsHolding()
{
	RemoveBallArrivingWidget();
}

void AMyPaperCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPaperCharacter, IsHolding);
}
