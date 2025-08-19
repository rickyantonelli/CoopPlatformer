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
		if (LockSprite)
		{
			LockSprite->SetVisibility(false);
		}
	}


	// TODO: this should only be happening if its activated, which we can keep track of with a trigger area and collision
	// for the current scope though (demo) we will just assume it is always activated
	
	//// Since this is an optional trigger area, if it doesnt exist we are always activated
	//if (ActivatedArea)
	//{
	//	Activated = false;
	//	// bind the OnActorBeginOverlap event to the ActivatedArea's BeginOverlap function
	//	ActivatedArea->OnComponentBeginOverlap.AddDynamic(this, &APassSwapActor::OnActivatedAreaOverlapBegin);

	//}
	//else
	//{
	//	Activated = true;
	//}
}

// Called every frame
void APassSwapActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && !BindingsSet)
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
		if (LockSprite)
		{
			LockSprite->SetVisibility(true);
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
		if (LockSprite)
		{
			LockSprite->SetVisibility(false);
		}
	}

	FirstSetActive = !FirstSetActive;
}

void APassSwapActor::OnPassActivated()
{
	if (Activated)
	{
		MulticastSwapActors();
	}
}

void APassSwapActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// DOREPLIFETIME(APassSwapActor, FirstSetActive);
}