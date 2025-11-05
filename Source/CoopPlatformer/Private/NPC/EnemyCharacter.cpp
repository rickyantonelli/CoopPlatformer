// Copyright Ricky Antonelli


#include "NPC/EnemyCharacter.h"
#include "PaperSpriteComponent.h"
#include "PaperFlipbookComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include <Components/TextRenderComponent.h>

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);

	Health = 5;
	MaxHealth = 5;
	CooldownTimer = 1;

	bCanDamage = true;
	bCanPatrol = true;
	bAwake = false;
	FollowType = EFollowType::Off;


	GetCapsuleComponent()->SetIsReplicated(true);
	MoveSpeed = 100.0f;

	GameStateRef = nullptr;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	MaxHealth = Health;

	GameStateRef = GetWorld()->GetGameState<AMyGameStateBase>();

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnEnemyOverlapped);
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

	if (!bAwake) return;

	if (bCanPatrol)
	{
		switch (FollowType)
		{
		case EFollowType::Off:
			break;
		case EFollowType::NearestPlayer:
			PatrolToNearest(DeltaTime);
			break;
		case EFollowType::Ball:
			PatrolToBall(DeltaTime);
			break;
		}
	}
}

void AEnemyCharacter::PatrolToNearest(float DeltaTime)
{
	if (GameStateRef && GameStateRef->ActivePlayers.Num() == 2)
	{
		if (!GameStateRef->ActivePlayers[0] || !GameStateRef->ActivePlayers[1]) return;
		FVector TargetLocation = FVector::ZeroVector;

		if (GameStateRef->ActivePlayers[0]->bDead)
		{
			if (GameStateRef->ActivePlayers[1]->bDead) return;
			TargetLocation = GameStateRef->ActivePlayers[1]->GetActorLocation();
		}
		else if (GameStateRef->ActivePlayers[1]->bDead)
		{
			TargetLocation = GameStateRef->ActivePlayers[0]->GetActorLocation();
		}
		else
		{

			// figure out which player is closest then move towards them
			FVector PlayerOneLocation = GameStateRef->ActivePlayers[0]->GetActorLocation();
			FVector PlayerTwoLocation = GameStateRef->ActivePlayers[1]->GetActorLocation();

			// get the closer player
			TargetLocation = (FVector::Dist(GetActorLocation(), PlayerOneLocation) < FVector::Dist(GetActorLocation(), PlayerTwoLocation)) ? PlayerOneLocation : PlayerTwoLocation;
		}
		FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
		AddMovementInput(Direction, MoveSpeed * DeltaTime);
	}
}

void AEnemyCharacter::PatrolToBall(float DeltaTime)
{
	if (GameStateRef && GameStateRef->BallActor)
	{
		FVector TargetLocation = GameStateRef->BallActor->GetActorLocation();
		FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
		AddMovementInput(Direction, MoveSpeed * DeltaTime);
	}
}

void AEnemyCharacter::OnEnemyOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() && OtherActor->ActorHasTag("Ball") && bCanDamage)
	{
		UE_LOG(LogTemp, Log, TEXT("APPLYING DAMAGE"));
		Health -= 1;
		bCanDamage = false;
		MulticastApplyDeath(Health);
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {bCanDamage = true; }, CooldownTimer, false);
	}
}

void AEnemyCharacter::OnTriggerBoxOverlapped(AActor* PlayerActor, AActor* OtherActor)
{
	if (bAwake) return;
	if (!HasAuthority()) return;
	if (OtherActor->ActorHasTag("Player"))
	{
		bAwake = true;
	}
}

void AEnemyCharacter::OnResetActivated()
{
	UTextRenderComponent* HealthText = FindComponentByClass<UTextRenderComponent>();
	if (HealthText)
	{
		float PercentHealth = 100.0f;
		FString HealthString = FString::Printf(TEXT("%.0f%%"), PercentHealth);
		HealthText->SetText(FText::FromString(HealthString));
	}

	if (!HasAuthority()) return;
	Health = MaxHealth;
	bAwake = false;
}

void AEnemyCharacter::DisableActors()
{
	// Disable Actors
	for (AActor* LockedActor : LockedActors)
	{
		if (!LockedActor) continue;
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
	UTextRenderComponent* HealthText = FindComponentByClass<UTextRenderComponent>();
	if (HealthText)
	{
		float PercentHealth = (static_cast<float>(NewHealth) / MaxHealth) * 100.0f;
		FString HealthString = FString::Printf(TEXT("%.0f%%"), PercentHealth);
		HealthText->SetText(FText::FromString(HealthString));
	}


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
	DOREPLIFETIME(AEnemyCharacter, bCanDamage);
	DOREPLIFETIME(AEnemyCharacter, LockedActors);
	DOREPLIFETIME(AEnemyCharacter, UnlockedActors);
	DOREPLIFETIME(AEnemyCharacter, bCanPatrol);
}