// Copyright Ricky Antonelli


#include "Mechanics/Keys/ExtendKeyActor.h"
#include "Net/UnrealNetwork.h"

AExtendKeyActor::AExtendKeyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	InTimer = false;
	UnlockTimer = 3.0f;
}

void AExtendKeyActor::BeginPlay()
{
	Super::BeginPlay();

	for (UActorComponent* Component : GetComponents())
	{
		UBoxComponent* MeshComponent = Cast<UBoxComponent>(Component);
		if (MeshComponent)
		{
			KeyMeshes.Add(MeshComponent);
			MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AExtendKeyActor::OnBoxCollision);
		}
	}
}

void AExtendKeyActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Ball") && Locked && LockedActors.Num() > 0 && !OverlappedMeshes.Contains(OverlappedComponent) && HasAuthority())
	{
        if (!InTimer)
        {
            GetWorld()->GetTimerManager().SetTimer(UnlockTimerHandle, [&]() { if (HasAuthority()) MulticastTimerExpired(); }, UnlockTimer, false);
            InTimer = true;
        }
        else
        {
            GetWorld()->GetTimerManager().ClearTimer(UnlockTimerHandle);
            GetWorld()->GetTimerManager().SetTimer(UnlockTimerHandle, [&]() { if (HasAuthority()) MulticastTimerExpired(); }, UnlockTimer, false);
        }

		OverlappedMeshes.Add(OverlappedComponent);

		if (OverlappedMeshes.Num() == KeyMeshes.Num())
		{
			if (HasAuthority()) MulticastTriggerUnlock();
			return;
		}

		UPaperSpriteComponent* SpriteComponent;
		SpriteComponent = Cast<UPaperSpriteComponent>(OverlappedComponent->GetChildComponent(0));
		if (SpriteComponent) MulticastYellowKey(SpriteComponent);
	}
}

void AExtendKeyActor::MulticastYellowKey_Implementation(UPaperSpriteComponent* SpriteComp)
{
	SpriteComp->SetSprite(YellowKey);
}

void AExtendKeyActor::MulticastTimerExpired_Implementation()
{
	if (Locked) OverlappedMeshes.Empty();
	for (UPaperSpriteComponent* SpriteComp : SpriteComps)
	{
		if (SpriteComp)
		{
			SpriteComp->SetSprite(RedKey);
		}
	}
}

void AExtendKeyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExtendKeyActor, OverlappedMeshes);
}