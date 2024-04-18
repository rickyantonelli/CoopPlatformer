// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeyActor.generated.h"

UCLASS()
class COOPPLATFORMER_API AKeyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AKeyActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Customizable")
	AActor* LockActor;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	USceneComponent* RootComp;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	bool Locked;

};
