// Copyright Ricky Antonelli


#include "Mechanics/Keys/PassKeyActor.h"

APassKeyActor::APassKeyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);
}

void APassKeyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APassKeyActor::BeginPlay()
{
	Super::BeginPlay();

	Box = GetComponentByClass<UBoxComponent>();
	if (Box)
	{
		Box->OnComponentBeginOverlap.AddDynamic(this, &APassKeyActor::OnBoxCollision);
	}

	Sprite = GetComponentByClass<UPaperSpriteComponent>();
	if (Sprite)
	{
		Sprite->OnComponentBeginOverlap.AddDynamic(this, &APassKeyActor::OnBoxCollision);
	}
}

void APassKeyActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Ball") && Locked && LockedActors.Num() > 0)
	{
		if (HasAuthority())
		{
			MulticastTriggerUnlock();
		}
	}
}

void APassKeyActor::MulticastYellowKey_Implementation(UPaperSpriteComponent* SpriteComp)
{
	SpriteComp->SetSprite(YellowKey);
}