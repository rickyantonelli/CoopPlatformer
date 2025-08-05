// Copyright Ricky Antonelli


#include "Mechanics/Triggers/CameraTriggerBox.h"
#include "Character/MyPaperCharacter.h"
#include "PaperSpriteComponent.h"
#include "Net/UnrealNetwork.h"

void ACameraTriggerBox::BeginPlay()
{
	Super::BeginPlay();

	UBoxComponent* CollisionComp = Cast<UBoxComponent>(GetCollisionComponent());
	if (CollisionComp)
	{
		CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ACameraTriggerBox::OnOverlapBegin);
		CollisionComp->OnComponentEndOverlap.AddDynamic(this, &ACameraTriggerBox::OnOverlapEnd);
	}
}

void ACameraTriggerBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

void ACameraTriggerBox::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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
