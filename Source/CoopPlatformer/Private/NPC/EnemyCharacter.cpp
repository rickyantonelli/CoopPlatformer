// Copyright Ricky Antonelli


#include "NPC/EnemyCharacter.h"
#include "PaperSpriteComponent.h"
#include "PaperFlipbookComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	bReplicates = true;
	bAlwaysRelevant = true;
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

	AController2D* MyController = Cast<AController2D>(GetWorld()->GetFirstPlayerController());
	if (MyController)
	{
		MyController->OnResetActivated.AddDynamic(this, &AEnemyCharacter::OnResetActivated);
	}
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
	if (HasAuthority() && OtherActor->ActorHasTag("Ball") && CanDamage)
	{
		UE_LOG(LogTemp, Log, TEXT("APPLYING DAMAGE"));
		Health -= 1;
		CanDamage = false;
		MulticastApplyDeath(Health);
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {CanDamage = true; }, CooldownTimer, false);
	}
}

void AEnemyCharacter::OnResetActivated()
{
	// the players are resetting, so we should reset as well
	// obviously long term this should not just be `set the health back to a constant`
	Health = 5;
}

void AEnemyCharacter::DisableActors()
{
	// Disable Actors
	for (AActor* LockedActor : LockedActors)
	{
		UStaticMeshComponent* LockMesh = LockedActor->GetComponentByClass<UStaticMeshComponent>();
		if (LockMesh)
		{
			LockMesh->SetVisibility(false);
			LockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			UBoxComponent* LockBox = LockedActor->GetComponentByClass<UBoxComponent>();
			UPaperSpriteComponent* LockSprite = LockedActor->GetComponentByClass<UPaperSpriteComponent>();
			if (LockBox)
			{
				LockBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			if (LockSprite)
			{
				LockSprite->SetVisibility(false);

			}
		}
	}
}

void AEnemyCharacter::EnableActors()
{
	// Enable Actors
	for (AActor* UnlockedActor : UnlockedActors)
	{
		UStaticMeshComponent* LockMesh = UnlockedActor->GetComponentByClass<UStaticMeshComponent>();
		if (LockMesh)
		{
			LockMesh->SetVisibility(true);
			LockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		else
		{
			UBoxComponent* LockBox = UnlockedActor->GetComponentByClass<UBoxComponent>();
			UPaperSpriteComponent* LockSprite = UnlockedActor->GetComponentByClass<UPaperSpriteComponent>();
			if (LockBox)
			{
				LockBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}
			if (LockSprite)
			{
				LockSprite->SetVisibility(true);
			}
		}
	}
}

void AEnemyCharacter::MulticastApplyDeath_Implementation(int32 NewHealth)
{
	if (DamageSound)
	{
		UGameplayStatics::PlaySound2D(this, DamageSound);
	}

	if (NewHealth <= 0)
	{
		DisableActors();
		EnableActors();
		Destroy();
	}
}

void AEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEnemyCharacter, Health);
	DOREPLIFETIME(AEnemyCharacter, CanDamage);
	DOREPLIFETIME(AEnemyCharacter, LockedActors);
	DOREPLIFETIME(AEnemyCharacter, UnlockedActors);
}