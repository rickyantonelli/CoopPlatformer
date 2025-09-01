// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include <Components/BoxComponent.h>
#include <PaperSpriteComponent.h>
#include "Character/MyPaperCharacter.h"
#include "GameFramework/Actor.h"
#include "FreezeToken.generated.h"

UCLASS()
class COOPPLATFORMER_API AFreezeToken : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFreezeToken();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void CollectToken();

	UFUNCTION()
	void SetCollectable();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<UPaperSpriteComponent> Sprite;

	UPROPERTY(EditAnywhere, Category = "Customizable")
	float RespawnTimer;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool CanBeCollected;

	UFUNCTION()
	void OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
