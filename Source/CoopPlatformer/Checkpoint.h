// Fill out your copyright notice in the Description page of Project Settings.

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
	// Sets default values for this actor's properties
	ACheckpoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	USceneComponent* RootComp; 

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* TriggerMesh;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	TArray<AMyPaperCharacter*> CheckpointedPlayers;

	UFUNCTION()
	void AddPlayer(AMyPaperCharacter* PlayerActor);

	bool CanBeCollected;


};
