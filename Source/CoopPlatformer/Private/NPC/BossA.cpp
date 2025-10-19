// Copyright Ricky Antonelli

#include "GameFramework/ProjectileMovementComponent.h"
#include "NPC/BossA.h"

ABossA::ABossA()
{
	WallProjectileInterval = 10.f;
	bWallProjectileRight = true;

	WallProjectile = nullptr;
}

void ABossA::BeginPlay()
{
	Super::BeginPlay();

	// start the wall projectile timer
	GetWorld()->GetTimerManager().SetTimer(WallProjectileTimerHandle, this, &ABossA::FireWallProjectile, WallProjectileInterval, false);
}

void ABossA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABossA::FireWallProjectile()
{
	if (!HasAuthority()) return;
	if (!WallProjectileClass) return;

	if (WallProjectile)
	{
		WallProjectile->Destroy();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	// spawn a wall projectile
	WallProjectile = GetWorld()->SpawnActor<AActor>(WallProjectileClass, GetActorLocation(), GetActorRotation(), SpawnParams);


	if (WallProjectile)
	{

		UProjectileMovementComponent* ProjectileMovement = WallProjectile->FindComponentByClass<UProjectileMovementComponent>();
		if (ProjectileMovement)
		{
			// Set initial velocity (in X direction for your 2D setup)
			FVector LaunchVelocity = bWallProjectileRight ? FVector(ProjectileMovement->InitialSpeed, 0.f, 0.f) : FVector(-ProjectileMovement->InitialSpeed, 0.f, 0.f);
			ProjectileMovement->Velocity = LaunchVelocity;
		}
	}

	bWallProjectileRight = !bWallProjectileRight;

	GetWorld()->GetTimerManager().SetTimer(WallProjectileTimerHandle, this, &ABossA::FireWallProjectile, WallProjectileInterval, false);
}
