// Copyright Ricky Antonelli


#include "Mechanics/Movement/PassSwapActor.h"
#include "Character/MyPaperCharacter.h"
#include "Systems/CoopPlatformerGameModeBase.h"
#include "PaperSpriteComponent.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
APassSwapActor::APassSwapActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

}

// Called when the game starts or when spawned
void APassSwapActor::BeginPlay()
{
	Super::BeginPlay();


	 // hide all actors in the second set
	for (AActor* Actor : SecondSetActors)
	{
		UBoxComponent* LockBox = Actor->GetComponentByClass<UBoxComponent>();
		UPaperSpriteComponent* LockSprite = Actor->GetComponentByClass<UPaperSpriteComponent>();

		if (LockBox)
		{
			LockBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (LockSprite && OffSprite)
		{
			LockSprite->SetSprite(OffSprite);
			if (!LockBox)
			{
				LockSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}

	if (ActivationArea)
	{
		ActivationArea->OnActorBeginOverlap.AddDynamic(this, &APassSwapActor::OnActivateTriggerBeginOverlap);
		ActivationArea->OnActorEndOverlap.AddDynamic(this, &APassSwapActor::OnActivateTriggerEndOverlap);
	}
}

// Called every frame
void APassSwapActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;

	if (!BindingsSet)
	{
		TArray<AActor*> Controllers;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), Controllers);

		if (Controllers.Num() == 2)
		{
			for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
			{
				AController2D* PC = Cast<AController2D>(*Iterator);
				if (PC)
				{
					PC->OnPassActivated.AddDynamic(this, &APassSwapActor::OnPassActivated);
				}
			}
			BindingsSet = true;
		}
	}
}

void APassSwapActor::MulticastSwapActors_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("MulticastSwapActors called. FirstSetActive: %s"), FirstSetActive ? TEXT("true") : TEXT("false"));
	const TArray<AActor*>& ActivatingActors = FirstSetActive ? SecondSetActors : FirstSetActors;
	const TArray<AActor*>& DisablingActors = FirstSetActive ? FirstSetActors : SecondSetActors;
	UE_LOG(LogTemp, Log, TEXT("Swapping actors"));
	for (AActor* ActivatingActor : ActivatingActors)
	{
		UBoxComponent* LockBox = ActivatingActor->GetComponentByClass<UBoxComponent>();
		UPaperSpriteComponent* LockSprite = ActivatingActor->GetComponentByClass<UPaperSpriteComponent>();
		if (LockBox)
		{
			LockBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}

		if (LockSprite && OnSprite)
		{
			LockSprite->SetSprite(OnSprite);
			if (!LockBox)
			{
				LockSprite->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}
		}
	}

	for (AActor* DisablingActor : DisablingActors)
	{
		UBoxComponent* LockBox = DisablingActor->GetComponentByClass<UBoxComponent>();
		UPaperSpriteComponent* LockSprite = DisablingActor->GetComponentByClass<UPaperSpriteComponent>();
		if (LockBox)
		{
			LockBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (LockSprite && OffSprite)
		{
			LockSprite->SetSprite(OffSprite);
			if (!LockBox)
			{
				LockSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}

	FirstSetActive = !FirstSetActive;
}

void APassSwapActor::OnPassActivated()
{
	if (bActivated)
	{
		MulticastSwapActors();
	}
}

void APassSwapActor::OnActivateTriggerBeginOverlap(AActor* PlayerActor, AActor* OtherActor)
{
	if (!HasAuthority()) return;
	if (!OtherActor->ActorHasTag("Player")) return;

	if (!CurrentActiveActors.Contains(OtherActor))
	{
		CurrentActiveActors.Add(OtherActor);
	}

	if (CurrentActiveActors.Num() == 2)
	{
		bActivated = true;
	}
}

void APassSwapActor::OnActivateTriggerEndOverlap(AActor* PlayerActor, AActor* OtherActor)
{
	if (!HasAuthority()) return;
	if (!OtherActor->ActorHasTag("Player")) return;

	if (CurrentActiveActors.Contains(OtherActor))
	{
		CurrentActiveActors.Remove(OtherActor);
	}

	if (CurrentActiveActors.Num() < 2)
	{
		bActivated = false;
	}
}

void APassSwapActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APassSwapActor, bActivated);
	DOREPLIFETIME(APassSwapActor, CurrentActiveActors);
}