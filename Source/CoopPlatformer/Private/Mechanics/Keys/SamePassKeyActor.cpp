// Copyright Ricky Antonelli


#include "Mechanics/Keys/SamePassKeyActor.h"
#include "PaperFlipbookComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ASamePassKeyActor::ASamePassKeyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);
}

void ASamePassKeyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASamePassKeyActor, OverlappedMeshes);
}

void ASamePassKeyActor::Tick(float DeltaSeconds)
{
	if (!Locked) return;

	if (OverlappedMeshes.Num() == KeyMeshes.Num())
	{
		if (HasAuthority() && Locked) MulticastTriggerUnlock();
	}

	Super::Tick(DeltaSeconds);
}

void ASamePassKeyActor::BeginPlay()
{
	Super::BeginPlay();

	for (UPaperSpriteComponent* SpriteComponent : SpriteComps)
	{
		if (SpriteComponent)
		{
			KeyMeshes.Add(SpriteComponent);
			SpriteComponent->OnComponentBeginOverlap.AddDynamic(this, &ASamePassKeyActor::OnBoxCollision);
		}
	}
}

UPaperFlipbookComponent* ASamePassKeyActor::FindFlipbookForSprite(UPaperSpriteComponent* SpriteComp) const
{
	if (!SpriteComp)
	{
		return nullptr;
	}

	TArray<USceneComponent*> ChildComponents;
	SpriteComp->GetChildrenComponents(false, ChildComponents);
	for (USceneComponent* ChildComponent : ChildComponents)
	{
		if (UPaperFlipbookComponent* FlipbookComp = Cast<UPaperFlipbookComponent>(ChildComponent))
		{
			return FlipbookComp;
		}
	}

	if (USceneComponent* AttachParent = SpriteComp->GetAttachParent())
	{
		TArray<USceneComponent*> SiblingComponents;
		AttachParent->GetChildrenComponents(false, SiblingComponents);
		for (USceneComponent* SiblingComponent : SiblingComponents)
		{
			if (UPaperFlipbookComponent* FlipbookComp = Cast<UPaperFlipbookComponent>(SiblingComponent))
			{
				return FlipbookComp;
			}
		}
	}

	return nullptr;
}

void ASamePassKeyActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() && CanBallActivateKey(OtherActor) && Locked && LockedActors.Num() > 0 && !OverlappedMeshes.Contains(OverlappedComponent))
	{
		OverlappedMeshes.Add(OverlappedComponent);

		UPaperSpriteComponent* SpriteComponent = Cast<UPaperSpriteComponent>(OverlappedComponent);
		UPaperFlipbookComponent* YellowFlipbookComponent = nullptr;
		if (SpriteComponent)
		{
			YellowFlipbookComponent = FindFlipbookForSprite(SpriteComponent);
			MulticastYellowKey(SpriteComponent, YellowFlipbookComponent);
		}
	}
}

void ASamePassKeyActor::MulticastYellowKey_Implementation(UPaperSpriteComponent* SpriteComp, UPaperFlipbookComponent* FlipbookComp)
{
	if (!SpriteComp)
	{
		return;
	}

	if (!YellowFlipbook || !FlipbookComp)
	{
		SpriteComp->SetSprite(YellowKey);
		return;
	}

	PendingYellowTransitions.Add(FlipbookComp, SpriteComp);
	FlipbookComp->OnFinishedPlaying.RemoveDynamic(this, &AKeyActor::OnUnlockFlipbookFinished);
	FlipbookComp->OnFinishedPlaying.AddUniqueDynamic(this, &ASamePassKeyActor::OnYellowFlipbookFinished);

	SpriteComp->SetVisibility(false);
	FlipbookComp->SetFlipbook(YellowFlipbook);
	FlipbookComp->SetLooping(false);
	FlipbookComp->SetVisibility(true);
	FlipbookComp->PlayFromStart();
}

void ASamePassKeyActor::OnYellowFlipbookFinished()
{
	// The amount of flipbooks is arbitrary, so we have to do a slightly more complex approach than a traditional 
	// key actor unlock - but nothing too crazy
	for (auto TransitionIt = PendingYellowTransitions.CreateIterator(); TransitionIt; ++TransitionIt)
	{
		UPaperFlipbookComponent* FlipbookComp = TransitionIt.Key();
		UPaperSpriteComponent* SpriteComp = TransitionIt.Value();

		if (!IsValid(FlipbookComp) || !IsValid(SpriteComp))
		{
			TransitionIt.RemoveCurrent();
			continue;
		}

		if (FlipbookComp->IsPlaying())
		{
			continue;
		}

		if (FlipbookComp->GetFlipbook() == YellowFlipbook)
		{
			SpriteComp->SetSprite(YellowKey);
			SpriteComp->SetVisibility(true);
			FlipbookComp->SetVisibility(false);
		}

		FlipbookComp->OnFinishedPlaying.RemoveDynamic(this, &ASamePassKeyActor::OnYellowFlipbookFinished);
		FlipbookComp->OnFinishedPlaying.AddUniqueDynamic(this, &AKeyActor::OnUnlockFlipbookFinished);
		TransitionIt.RemoveCurrent();
	}
}

void ASamePassKeyActor::MulticastRedKey_Implementation()
{
	for (UPaperSpriteComponent* SpriteComp : SpriteComps)
	{
		if (!SpriteComp)
		{
			continue;
		}

		if (UPaperFlipbookComponent* FlipbookComp = FindFlipbookForSprite(SpriteComp))
		{
			FlipbookComp->Stop();
			FlipbookComp->SetVisibility(false);
			FlipbookComp->OnFinishedPlaying.RemoveDynamic(this, &AKeyActor::OnUnlockFlipbookFinished);
			FlipbookComp->OnFinishedPlaying.RemoveDynamic(this, &ASamePassKeyActor::OnYellowFlipbookFinished);
			FlipbookComp->OnFinishedPlaying.RemoveDynamic(this, &ASamePassKeyActor::OnSamePassUnlockFlipbookFinished);
		}

		SpriteComp->SetSprite(RedKey);
		SpriteComp->SetVisibility(true);
	}

	PendingYellowTransitions.Empty();
	PendingUnlockTransitions.Empty();
}

void ASamePassKeyActor::MulticastTriggerUnlock_Implementation()
{
	Locked = false;

	for (AActor* LockedActor : LockedActors)
	{
		UStaticMeshComponent* LockMesh = LockedActor->GetComponentByClass<UStaticMeshComponent>();
		if (LockMesh)
		{
			LockMesh->SetVisibility(false);
			LockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			continue;
		}

		UBoxComponent* LockBox = LockedActor->GetComponentByClass<UBoxComponent>();
		UPaperSpriteComponent* LockSprite = LockedActor->GetComponentByClass<UPaperSpriteComponent>();
		UPaperFlipbookComponent* LockFlipbook = LockedActor->GetComponentByClass<UPaperFlipbookComponent>();
		if (LockBox)
		{
			LockBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (LockSprite)
		{
			LockSprite->SetVisibility(false);
			LockSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (LockFlipbook)
		{
			LockFlipbook->SetVisibility(true);
			LockFlipbook->SetLooping(false);
			LockFlipbook->PlayFromStart();
			LockFlipbook->OnFinishedPlaying.AddUniqueDynamic(this, &AKeyActor::OnDoorFlipbookFinished);
		}
	}

	for (UPaperSpriteComponent* SpriteComp : SpriteComps)
	{
		if (!SpriteComp)
		{
			continue;
		}

		UPaperFlipbookComponent* FlipbookComp = FindFlipbookForSprite(SpriteComp);
		if (!UnlockFlipbook || !FlipbookComp)
		{
			SpriteComp->SetSprite(GreenKey);
			SpriteComp->SetVisibility(true);
			continue;
		}

		PendingYellowTransitions.Remove(FlipbookComp);
		PendingUnlockTransitions.Add(FlipbookComp, SpriteComp);
		FlipbookComp->OnFinishedPlaying.RemoveDynamic(this, &AKeyActor::OnUnlockFlipbookFinished);
		FlipbookComp->OnFinishedPlaying.RemoveDynamic(this, &ASamePassKeyActor::OnYellowFlipbookFinished);
		FlipbookComp->OnFinishedPlaying.AddUniqueDynamic(this, &ASamePassKeyActor::OnSamePassUnlockFlipbookFinished);

		SpriteComp->SetVisibility(false);
		FlipbookComp->SetFlipbook(UnlockFlipbook);
		FlipbookComp->SetLooping(false);
		FlipbookComp->SetVisibility(true);
		FlipbookComp->PlayFromStart();
	}

	if (CompletedSound)
	{
		UGameplayStatics::PlaySound2D(this, CompletedSound);
	}
}

void ASamePassKeyActor::OnSamePassUnlockFlipbookFinished()
{
	for (auto TransitionIt = PendingUnlockTransitions.CreateIterator(); TransitionIt; ++TransitionIt)
	{
		UPaperFlipbookComponent* FlipbookComp = TransitionIt.Key();
		UPaperSpriteComponent* SpriteComp = TransitionIt.Value();
		if (!IsValid(FlipbookComp) || !IsValid(SpriteComp))
		{
			TransitionIt.RemoveCurrent();
			continue;
		}

		if (FlipbookComp->IsPlaying())
		{
			continue;
		}

		if (FlipbookComp->GetFlipbook() == UnlockFlipbook)
		{
			SpriteComp->SetSprite(GreenKey);
			SpriteComp->SetVisibility(true);
			FlipbookComp->SetVisibility(false);
		}

		FlipbookComp->OnFinishedPlaying.RemoveDynamic(this, &ASamePassKeyActor::OnSamePassUnlockFlipbookFinished);
		TransitionIt.RemoveCurrent();
	}
}

void ASamePassKeyActor::OnBallCaught()
{
	// Only care to do this if we still need to unlock this
	if (Locked)
	{
		OverlappedMeshes.Empty();
		MulticastRedKey();
	}
}
