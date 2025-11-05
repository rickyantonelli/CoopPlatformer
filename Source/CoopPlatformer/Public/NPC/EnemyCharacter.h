// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Controller/Controller2D.h"
#include "Components/ArrowComponent.h"
#include "Systems/MyGameStateBase.h"
#include "EnemyCharacter.generated.h"

class USoundBase;

UENUM(BlueprintType)
enum class EFollowType : uint8
{
	Off				UMETA(DisplayName = "Off"),
	NearestPlayer   UMETA(DisplayName = "NearestPlayer"),
	Ball			UMETA(DisplayName = "Ball"),
};

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

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	EFollowType FollowType;

	UPROPERTY(EditAnywhere, Category = "Customizable Values")
	TObjectPtr<AActor> TriggerBox;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool bCanDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<AActor*> LockedActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<AActor*> UnlockedActors;

	UFUNCTION()
	virtual void PatrolToNearest(float DeltaTime);

	UFUNCTION()
	virtual void PatrolToBall(float DeltaTime);

	UFUNCTION()
	void OnEnemyOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnTriggerBoxOverlapped(AActor* PlayerActor, AActor* OtherActor);

	UFUNCTION()
	virtual void OnResetActivated();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyDeath(int32 NewHealth);

	UFUNCTION()
	void DisableActors();

	UFUNCTION()
	void EnableActors();

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<USoundBase> DamageSound;

	TObjectPtr<AMyGameStateBase> GameStateRef;

	UPROPERTY(Replicated)
	bool bCanPatrol;

	UPROPERTY(Replicated)
	bool bAwake;

	int MaxHealth;
};
