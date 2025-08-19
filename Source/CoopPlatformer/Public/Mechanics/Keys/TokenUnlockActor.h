// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperSpriteComponent.h"
#include "Components/BoxComponent.h"
#include "Character/MyPaperCharacter.h"
#include "TokenUnlockActor.generated.h"

UCLASS()
class COOPPLATFORMER_API ATokenUnlockActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATokenUnlockActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Customizable")
	TObjectPtr<UPaperSpriteComponent> Sprite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customizable")
	TArray<AActor*> LockedActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customizable")
	bool bLandedToken = false;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	AMyPaperCharacter* CollectingCharacter;

	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastUnlockActors();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastResetToken();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHideToken();

	bool bCollected = false;
};
