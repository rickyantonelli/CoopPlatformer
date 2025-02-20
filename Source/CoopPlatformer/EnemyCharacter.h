// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Components/ArrowComponent.h"
#include "EnemyCharacter.generated.h"

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

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Debug")
	bool CanDamage;

	UFUNCTION()
	virtual void Patrol();

	UFUNCTION()
	void OnOverlapBegin(AActor* PlayerActor, AActor* OtherActor);

	UFUNCTION()
	void OnOverlapEnd(AActor* PlayerActor, AActor* OtherActor);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyDeath();

	/** The first point that the actor moves to */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UArrowComponent> Point1;

	/** The second point that the actor moves to */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UArrowComponent> Point2;

	UPROPERTY(EditAnywhere, Replicated)
	FVector StartPoint;

	/** The point that the actor should move towards */
	UPROPERTY(EditAnywhere, Replicated)
	FVector EndPoint;
};
