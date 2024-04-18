// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/ArrowComponent.h"
#include "Transporter.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPPLATFORMER_API UTransporter : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTransporter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
	FVector StartPoint;

	UPROPERTY(EditAnywhere)
	FVector EndPoint;
	
	UPROPERTY(EditAnywhere, Category = "Customizable")
	float MoveSpeed;

	UFUNCTION(BlueprintCallable)
	void SetPoints(FVector Point1, FVector Point2);
};
