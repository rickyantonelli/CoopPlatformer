// Copyright Ricky Antonelli


#include "Mechanics/Triggers/BallReboundComponent.h"
#include <Kismet/GameplayStatics.h>
#include "Components/BoxComponent.h"


// Sets default values for this component's properties
UBallReboundComponent::UBallReboundComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UBallReboundComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner() || bControllersBound) return;
	if (!GetOwner()->HasAuthority()) return;

	UPaperSpriteComponent* Sprite = GetOwner()->FindComponentByClass<UPaperSpriteComponent>();
	if (Sprite)
	{
		Sprite->OnComponentBeginOverlap.AddDynamic(this, &UBallReboundComponent::OnOverlapBegin);
	}

	UBoxComponent* Box = GetOwner()->FindComponentByClass<UBoxComponent>();
	if (Box)
	{
		Box->OnComponentBeginOverlap.AddDynamic(this, &UBallReboundComponent::OnOverlapBegin);
	}

	
}


// Called every frame
void UBallReboundComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!AuthController)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AController2D* MyController = Cast<AController2D>(It->Get());
			if (MyController && MyController->HasAuthority() && MyController->BallActor)
			{
				AuthController = MyController;
				break;
			}
		}
	}
}

void UBallReboundComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->ActorHasTag("Ball")) return;

	ABallActor* Ball = Cast<ABallActor>(OtherActor);
	if (!Ball) return;
	if (Ball->IsAttached) return;
	if (!AuthController) return;

	// Spawn spark effect on all clients
	if (SparkActorClass)
	{
		FVector SpawnLocation = SweepResult.ImpactPoint;
		if (SpawnLocation.IsZero())
		{
			SpawnLocation = OtherActor->GetActorLocation();
		}

		AActor* SparkActor = GetWorld()->SpawnActor<AActor>(SparkActorClass, SpawnLocation, FRotator::ZeroRotator);
		if (SparkActor)
		{
			SparkActor->SetLifeSpan(SparkLifetime);
		}
	}

	AuthController->ReturnBallToThrower();

}

void UBallReboundComponent::MulticastSpawnSparkEffect_Implementation(FVector SpawnLocation)
{
	if (SparkActorClass)
	{
		AActor* SparkActor = GetWorld()->SpawnActor<AActor>(SparkActorClass, SpawnLocation, FRotator::ZeroRotator);
		if (SparkActor)
		{
			SparkActor->SetLifeSpan(SparkLifetime);
		}
	}
}

