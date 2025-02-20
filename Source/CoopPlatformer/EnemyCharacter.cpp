// Copyright Ricky Antonelli


#include "EnemyCharacter.h"
#include "Net/UnrealNetwork.h"

AEnemyCharacter::AEnemyCharacter()
{
	SetReplicates(true);
	bReplicates = true;

	// the enemy health, we dont need to keep track of a damage amount since it will always be 1
	Health = 5;
	// whether we can damage the enemy - we want this because we dont want damage to be applied multiple times in short frames
	CanDamage = true;

	Point1 = CreateDefaultSubobject<UArrowComponent>(TEXT("Point1"));
	Point1->SetupAttachment(GetRootComponent());
	Point1->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	Point2 = CreateDefaultSubobject<UArrowComponent>(TEXT("Point2"));
	Point2->SetupAttachment(GetRootComponent());
	Point2->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));

	MoveSpeed = 100.0f;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	this->OnActorBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnOverlapBegin);
	StartPoint = GetActorLocation() + Point1->GetRelativeLocation();
	EndPoint = GetActorLocation() + Point2->GetRelativeLocation();
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
	{
		FVector CurrentLocation = GetActorLocation();
		FVector TargetLocation = EndPoint;
		if (!CurrentLocation.Equals(TargetLocation))
		{
			FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);
			SetActorLocation(NewLocation);
		}
		else
		{
			// if the current location is at the TargetLocation, flip the start and end point so that we go the other way
			FVector Temp = StartPoint;
			StartPoint = EndPoint;
			EndPoint = Temp;
		}
	}
	else
	{
		FVector CurrentLocation = GetActorLocation();
		FVector TargetLocation = EndPoint;
		if (!CurrentLocation.Equals(TargetLocation))
		{
			FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);
			SetActorLocation(NewLocation);
		}
	}
}

void AEnemyCharacter::Patrol()
{
	// for now we are just doing the basic code as if this is a moving platform
	// in the future we want enemy AI to be much more complex so we'll probably completely redo this

}

void AEnemyCharacter::OnOverlapBegin(AActor* PlayerActor, AActor* OtherActor)
{
	if (OtherActor->ActorHasTag("Ball") && CanDamage && HasAuthority())
	{
		Health -= 1;
		CanDamage = false;
		if (Health <= 0) MulticastApplyDeath();
	}
}

void AEnemyCharacter::OnOverlapEnd(AActor* PlayerActor, AActor* OtherActor)
{
	// for now as soon as overlap ends, we will restore the CanDamage - but tbd if this is the right approach
	if (OtherActor->ActorHasTag("Ball") && HasAuthority())
	{
		CanDamage = true;
	}
}

void AEnemyCharacter::MulticastApplyDeath_Implementation()
{
	Destroy();
}

void AEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEnemyCharacter, Health);
	DOREPLIFETIME(AEnemyCharacter, CanDamage);
}