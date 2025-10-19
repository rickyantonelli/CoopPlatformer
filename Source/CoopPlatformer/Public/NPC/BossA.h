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

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Category = "Custom|Wall Projectile")
	TSubclassOf<AActor> WallProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Custom|Wall Projectile")
	float WallProjectileInterval;

	FTimerHandle WallProjectileTimerHandle;

	TObjectPtr<AActor> WallProjectile;

	bool bWallProjectileRight;

	UFUNCTION()
	void FireWallProjectile();
	
};
