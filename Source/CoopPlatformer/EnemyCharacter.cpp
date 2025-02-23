// Copyright Ricky Antonelli


#include "EnemyCharacter.h"
#include "Net/UnrealNetwork.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	bReplicates = true;
	SetReplicateMovement(true);

	// the enemy health, we dont need to keep track of a damage amount since it will always be 1
	Health = 5;
	// whether we can damage the enemy - we want this because we dont want damage to be applied multiple times in short frames
	CanDamage = true;
	// cooldown for ball passes
	CooldownTimer = 1;

	GetCapsuleComponent()->SetIsReplicated(true);
	MoveSpeed = 100.0f;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnComponentOverlap);
	}

	//AController2D* MyController = Cast<AController2D>(GetWorld()->GetFirstPlayerController());
	//if (MyController)
	//{
	//	MyController->OnCaughtActivated.AddDynamic(this, &AEnemyCharacter::OnBallCaught);
	//}
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyCharacter::Patrol()
{
	// for now we are just doing the basic code as if this is a moving platform
	// in the future we want enemy AI to be much more complex so we'll probably completely redo this

}

void AEnemyCharacter::OnComponentOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Ball") && CanDamage && HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("APPLYING DAMAGE"));
		Health -= 1;
		CanDamage = false;
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {CanDamage = true; }, CooldownTimer, false);
		if (Health <= 0) MulticastApplyDeath();
	}
}

void AEnemyCharacter::OnBallCaught()
{
	// for now as soon as overlap ends, we will restore the CanDamage - but tbd if this is the right approach
	CanDamage = true;
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