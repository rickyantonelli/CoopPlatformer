// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyPaperCharacter.h"
#include "Checkpoint.generated.h"

UCLASS()
class COOPPLATFORMER_API ACheckpoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ACheckpoint();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void AddPlayer(AMyPaperCharacter* PlayerActor);

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	USceneComponent* RootComp; 

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* TriggerMesh;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	TArray<AMyPaperCharacter*> CheckpointedPlayers;
	
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool CanBeCollected;


};
