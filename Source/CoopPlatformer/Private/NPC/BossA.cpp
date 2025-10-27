// Copyright Ricky Antonelli

#include "NPC/BossA.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include <Kismet/GameplayStatics.h>

ABossA::ABossA()
{
	WallProjectileInterval = 10.f;
	WallDestroyInterval = 20.f;
	WallResumeInterval = 10.f;
	bWallProjectileRight = true;
	bMoveToOrigin = false;

	BulletProjectileInterval = 2.f;
	BulletDestroyInterval = 10.f;
	bBulletTargetFirstPlayer = true;
}

void ABossA::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnActorBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnTriggerBoxOverlapped);
	}

	ResumeTimers();

	OriginalLocation = GetActorLocation();
}

void ABossA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bMoveToOrigin)
	{
        if (FVector::Dist(GetActorLocation(), OriginalLocation) <= 5) // SMALL_NUMBER)
		{
			bMoveToOrigin = false;
			FireWallProjectile();
		}
		else
		{
			// move towards original location
			FVector Direction = (OriginalLocation - GetActorLocation()).GetSafeNormal();
			AddMovementInput(Direction, MoveSpeed * DeltaTime);
		}
	}
}

void ABossA::BeginWallMechanic()
{
	if (!HasAuthority()) return;
	bCanPatrol = false;
	bMoveToOrigin = true;
}

void ABossA::FireWallProjectile()
{
	if (!HasAuthority()) return;
	if (!WallProjectileClass) return;

	bWallProjectileRight = !bWallProjectileRight;

	// cancel the bullet projectile timer while firing wall projectile
	GetWorld()->GetTimerManager().ClearTimer(BulletProjectileTimerHandle);

	FVector Direction = bWallProjectileRight ? FVector(1.f, 0.f, 0.f) : FVector(-1.f, 0.f, 0.f);
	FRotator AimRotation = Direction.Rotation();
	FTransform SpawnTransform(AimRotation, GetActorLocation());
	AActor* SpawnedProjectile = GetWorld()->SpawnActorDeferred<AActor>(
		WallProjectileClass,
		SpawnTransform,
		this,
		GetInstigator(),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (!SpawnedProjectile)
		return;

	if (UProjectileMovementComponent* ProjectileMovement = SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>())
	{
		ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;
	}

	UGameplayStatics::FinishSpawningActor(SpawnedProjectile, SpawnTransform);

	FTimerHandle ResumeTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(ResumeTimerHandle, this, &ABossA::ResumeTimers, WallResumeInterval, false);

	// timer to destroy projectile
	FTimerHandle WallDestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(WallDestroyTimerHandle, [SpawnedProjectile]() {SpawnedProjectile->Destroy(); }, WallDestroyInterval, false);

	// TODO: Timer to start firing bullet projectiles again after wall mechanic ends
}


void ABossA::FireBulletProjectile()
{
	if (!HasAuthority() || !BulletProjectileClass || !GameStateRef || GameStateRef->ActivePlayers.Num() != 2)
		return;

	bBulletTargetFirstPlayer = !bBulletTargetFirstPlayer;

	AMyPaperCharacter* Player = bBulletTargetFirstPlayer ? GameStateRef->ActivePlayers[1] : GameStateRef->ActivePlayers[0];

	FVector Direction = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	Direction.Y = 0.f;
	Direction.Normalize();

	FRotator AimRotation = Direction.Rotation();

	FTransform SpawnTransform(AimRotation, GetActorLocation());
	AActor* SpawnedProjectile = GetWorld()->SpawnActorDeferred<AActor>(
		BulletProjectileClass,
		SpawnTransform,
		this,
		GetInstigator(),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (!SpawnedProjectile)
		return;

	if (UProjectileMovementComponent* ProjectileMovement = SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>())
	{
		ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;
	}

	UGameplayStatics::FinishSpawningActor(SpawnedProjectile, SpawnTransform);

	// timer for next projectile
	GetWorld()->GetTimerManager().SetTimer(BulletProjectileTimerHandle, this, &ABossA::FireBulletProjectile, BulletProjectileInterval, false);

	// timer to destroy projectile
	FTimerHandle BulletDestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(BulletDestroyTimerHandle, [SpawnedProjectile]() {SpawnedProjectile->Destroy(); }, BulletDestroyInterval, false);
}

void ABossA::ResumeTimers()
{
	if (!bAwake) return;
	if (!HasAuthority()) return;
	bCanPatrol = true;
	// start the wall projectile timer
	GetWorld()->GetTimerManager().SetTimer(WallProjectileTimerHandle, this, &ABossA::BeginWallMechanic, WallProjectileInterval, false);
	GetWorld()->GetTimerManager().SetTimer(BulletProjectileTimerHandle, this, &ABossA::FireBulletProjectile, BulletProjectileInterval, false);
}

void ABossA::OnTriggerBoxOverlapped(AActor* PlayerActor, AActor* OtherActor)
{
	Super::OnTriggerBoxOverlapped(PlayerActor, OtherActor);

	if (OtherActor->ActorHasTag("Player"))
	{
		ResumeTimers();
	}
}

void ABossA::OnResetActivated()
{
	Super::OnResetActivated();

	GetWorld()->GetTimerManager().ClearTimer(BulletProjectileTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(WallProjectileTimerHandle);

	SetActorLocation(OriginalLocation);
}

void ABossA::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABossA, bMoveToOrigin);
}