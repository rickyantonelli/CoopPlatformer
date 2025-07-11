// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Controller2D.h"
#include "Components/ArrowComponent.h"
#include "EnemyCharacter.generated.h"

class USoundBase;

/**
 * 
 */
UCLASS()
class COOPPLATFORMER_API AEnemyCharacter : public APaperZDCharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	virtual void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Replicated, Category = "Customizable Values")
	int Health;

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	float CooldownTimer;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool CanDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<AActor*> LockedActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<AActor*> UnlockedActors;

	UFUNCTION()
	virtual void Patrol();

	UFUNCTION()
	void OnComponentOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnResetActivated();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyDeath();

	UFUNCTION()
	void DisableActors();

	UFUNCTION()
	void EnableActors();

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<USoundBase> DamageSound;
};
