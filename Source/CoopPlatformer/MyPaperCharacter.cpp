// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPaperCharacter.h"
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

	MyCharacterMovement = GetCharacterMovement();

	IsHolding = false;
	CanJumpReset = false;

}

// Called when the game starts or when spawned
void AMyPaperCharacter::BeginPlay()
{
	Super::BeginPlay();

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
	// input is a Vector2D
	FVector MovementVector = Value.Get<FVector>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.X);
	}
}


void AMyPaperCharacter::Pass(const FInputActionValue& Value)
{
	if (IsHolding)
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
}

bool AMyPaperCharacter::CanJumpInternal_Implementation() const
{
	if (CanJumpReset) return true;
	else return Super::CanJumpInternal_Implementation();
}

void AMyPaperCharacter::Jump()
{
	Super::Jump();
}

void AMyPaperCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	CanJumpReset = false;
}