// Copyright Ricky Antonelli


#include "CameraTriggerBox.h"
#include "MyPaperCharacter.h"
#include "PaperSpriteComponent.h"
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
		if (Player && Player->SpringArm)
		{
			// Spring Arm Offset (if applicable)
			if (SpringArmOffset != 0.0f)
			{
				// Get the spring arm distance before changing, so we can revert back when leaving
				Player->SpringArm->TargetArmLength += SpringArmOffset;
			}

			// X Offset (TargetOffset)
			if (XOffset != 0.0f || ZOffset != 0.0f)
			{
				// Get the spring arm distance before changing, so we can revert back when leaving
				Player->SpringArm->TargetOffset = FVector(XOffset, 0.0f, ZOffset);
			}

			// Additionally get the background sprite, as this needs to move accordingly
			// TODO: Obviously taking the index is a bad idea, this is for demo purposes only
			if (Player->SpringArm->GetChildComponent(1))
			{
				UPaperSpriteComponent* Background = Cast<UPaperSpriteComponent>(Player->SpringArm->GetChildComponent(1));
				if (Background)
				{
					FVector RelativeLocation = Background->GetRelativeLocation();
					RelativeLocation.X += SpringArmOffset;
					RelativeLocation.Y += XOffset;
					RelativeLocation.Z += ZOffset;
					Background->SetRelativeLocation(RelativeLocation);
				}
			}
		}
	}
}

void ACameraTriggerBox::OnOverlapEnd(AActor* TriggerBoxActor, AActor* OtherActor)
{
	if (OtherActor->ActorHasTag("Player"))
	{
		AMyPaperCharacter* Player = Cast<AMyPaperCharacter>(OtherActor);
		if (Player && Player->SpringArm)
		{
			if (SpringArmOffset != 0.0f)
			{
				Player->SpringArm->TargetArmLength -= SpringArmOffset;
			}
			if (XOffset != 0.0f || ZOffset != 0.0f)
			{
				Player->SpringArm->TargetOffset = FVector(0.0f, 0.0f, 0.0f);
			}

			if (Player->SpringArm->GetChildComponent(1))
			{
				//Same for background sprite
				UPaperSpriteComponent* Background = Cast<UPaperSpriteComponent>(Player->SpringArm->GetChildComponent(1));
				if (Background)
				{
					FVector RelativeLocation = Background->GetRelativeLocation();
					RelativeLocation.X -= SpringArmOffset;
					RelativeLocation.Y -= XOffset;
					RelativeLocation.Z -= ZOffset;
					Background->SetRelativeLocation(RelativeLocation);
				}
			}
		}
	}
}
