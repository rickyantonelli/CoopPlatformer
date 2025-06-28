// Copyright Ricky Antonelli


#include "CameraTriggerBox.h"
#include "MyPaperCharacter.h"
#include "Net/UnrealNetwork.h"

void ACameraTriggerBox::BeginPlay()
{
	Super::BeginPlay();
	OnActorBeginOverlap.AddDynamic(this, &ACameraTriggerBox::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &ACameraTriggerBox::OnOverlapEnd);
}

void ACameraTriggerBox::OnOverlapBegin(AActor* TriggerBoxActor, AActor* OtherActor)
{
	if (OtherActor->ActorHasTag("Player"))
	{
		AMyPaperCharacter* Player = Cast<AMyPaperCharacter>(OtherActor);
		if (Player)
		{
			// Get the spring arm distance before changing, so we can revert back when leaving
			OriginalSpringArmDistance = Player->SpringArm->TargetArmLength;
			Player->SpringArm->TargetArmLength = SpringArmDistance;
		}
	}
}

void ACameraTriggerBox::OnOverlapEnd(AActor* TriggerBoxActor, AActor* OtherActor)
{
	if (OtherActor->ActorHasTag("Player"))
	{
		AMyPaperCharacter* Player = Cast<AMyPaperCharacter>(OtherActor);
		if (Player)
		{
			Player->SpringArm->TargetArmLength = OriginalSpringArmDistance;
		}
	}
}
