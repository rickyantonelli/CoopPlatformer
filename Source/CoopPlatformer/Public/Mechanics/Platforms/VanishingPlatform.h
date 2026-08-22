// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "VanishingPlatform.generated.h"

UCLASS()
class COOPPLATFORMER_API AVanishingPlatform : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVanishingPlatform();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> Platform;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UPaperSpriteComponent> Sprite;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UPaperFlipbookComponent> Flipbook;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customizable")
	TObjectPtr<UPaperFlipbook> VanishAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customizable")
	TObjectPtr<UPaperFlipbook> UnvanishAnimation = nullptr;

	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	bool bIsVanishing = false;
	bool bIsRespawning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customizable")
	float VanishingTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customizable")
	float RespawnTime = 2.0f;

	UFUNCTION()
	void ResetVanish();

	UFUNCTION()
	void OnVanishFlipbookFinished();

private:
	void SetFlipbookPlayRateForDuration(float Duration);

};
