// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "NPC/EnemyCharacter.h"
#include "BossA.generated.h"

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API ABossA : public AEnemyCharacter
{
	GENERATED_BODY()

public:
	ABossA();

	virtual void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Category = "Custom|Wall Projectile")
	TSubclassOf<AActor> WallProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Custom|Wall Projectile")
	float WallProjectileInterval;

	UPROPERTY(EditAnywhere, Category = "Custom|Wall Projectile")
	float WallDestroyInterval;

	bool bWallProjectileRight;

	UPROPERTY(EditAnywhere, Category = "Custom|Bullet Projectile")
	TSubclassOf<AActor> BulletProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Custom|Bullet Projectile")
	float BulletProjectileInterval;

	UPROPERTY(EditAnywhere, Category = "Custom|Bullet Projectile")
	float BulletDestroyInterval;

	bool bBulletTargetFirstPlayer;

	FTimerHandle WallProjectileTimerHandle;

	FTimerHandle BulletProjectileTimerHandle;

	FVector OriginalLocation;

	UFUNCTION()
	void FireWallProjectile();

	UFUNCTION()
	void FireBulletProjectile();

};
