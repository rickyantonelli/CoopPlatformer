// Copyright Ricky Antonelli


#include "Mechanics/Triggers/BallReboundComponent.h"
#include <Kismet/GameplayStatics.h>
#include "Controller/Controller2D.h"
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

	UBoxComponent* Box = GetOwner()->FindComponentByClass<UBoxComponent>();
	if (!Box) return;

	Box->OnComponentBeginOverlap.AddDynamic(this, &UBallReboundComponent::OnOverlapBegin);
}


// Called every frame
void UBallReboundComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UBallReboundComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->ActorHasTag("Ball")) return;

	AController2D* MyController = Cast<AController2D>(GetWorld()->GetFirstPlayerController());
	if (!MyController) return;
	MyController->ReturnBallToThrower();

}

