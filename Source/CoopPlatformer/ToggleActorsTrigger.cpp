// Fill out your copyright notice in the Description page of Project Settings.


#include "ToggleActorsTrigger.h"

void AToggleActorsTrigger::OnOverlapBegin(AActor* TriggerBoxActor, AActor* OtherActor)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Overlap");
	if (SingleActor->GetParentActor())
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, SingleActor->GetParentActor()->GetActorLabel());
	}
}

void AToggleActorsTrigger::OnOverlapEnd(AActor* TriggerBoxActor, AActor* OtherActor)
{

}



void AToggleActorsTrigger::BeginPlay()
{
	Super::BeginPlay();
	OnActorBeginOverlap.AddDynamic(this, &AToggleActorsTrigger::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &AToggleActorsTrigger::OnOverlapEnd);

}