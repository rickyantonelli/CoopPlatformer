// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPaperCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

AMyPaperCharacter::AMyPaperCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	//SpringArm->SetupAttachment(RootComponent);
	//SpringArm->bUsePawnControlRotation = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->SetupAttachment(RootComponent);


	IsHolding = false;
	CanJumpReset = false;
	MovementEnabled = true;
	WithinCoyoteTime = false;
	Jumping = false;

	DeathDuration = 1.0f;
	CoyoteDuration = 0.5f;
}

// Called when the game starts or when spawned
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

// Called every frame
void AMyPaperCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMyPaperCharacter::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	EMovementMode NewMovementMode = GetCharacterMovement()->MovementMode;
	if (NewMovementMode == EMovementMode::MOVE_Falling)
	{
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {WithinCoyoteTime = true; }, CoyoteDuration, false);
	}

}

// Called to bind functionality to input
void AMyPaperCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AMyPaperCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

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
			// add movement 
			AddMovementInput(GetActorForwardVector(), MovementVector.X);
		}
	}
}


void AMyPaperCharacter::Pass(const FInputActionValue& Value)
{
	if (IsHolding && MovementEnabled)
	{
		OnPassActivated.Broadcast();
	}
}

void AMyPaperCharacter::ResetJumpAbility()
{
	CanJumpReset = true;
	IsHolding = true;
	if (HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, "SERVER");
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Purple, "CLIENT");
	}
}

void AMyPaperCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	CanJumpReset = false;	
	Jumping = false;
}

bool AMyPaperCharacter::CanJumpInternal_Implementation() const
{
	if (CanJumpReset || WithinCoyoteTime && !Jumping) 
	{
		return true;
	}

	else return Super::CanJumpInternal_Implementation();
}

void AMyPaperCharacter::Jump()
{
	if (MovementEnabled)
	{
		Super::Jump();
	}
	
}

void AMyPaperCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	CanJumpReset = false;
	Jumping = true;
}

void AMyPaperCharacter::OnDeath()
{
	if (IsLocallyControlled())
	{
		MovementEnabled = false;
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {MovementEnabled = true; }, DeathDuration, false);
	}
}