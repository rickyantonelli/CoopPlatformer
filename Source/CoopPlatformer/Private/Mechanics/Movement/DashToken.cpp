// Copyright Ricky Antonelli


#include "Mechanics/Movement/DashToken.h"
#include "Character/MyPaperCharacter.h"

// Sets default values
ADashToken::ADashToken()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);
	Mesh->SetCollisionProfileName(FName("OverlapAllDynamic"));

	RespawnTimer = 2.0f;

	CanBeCollected = true;

}
void ADashToken::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADashToken::Tick(float DeltaTime)
{
	//Mesh->AddRelativeRotation(FRotator(0.0f, 100.0f * DeltaTime, 0.0f));

	Super::Tick(DeltaTime);
}

void ADashToken::CollectDash()
{
	if (CanBeCollected)
	{
		CanBeCollected = false;
		Mesh->SetVisibility(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


		// after the timer, allow to be collected again
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {SetCollectable(); }, RespawnTimer, false);
	}
}

void ADashToken::SetCollectable()
{
	CanBeCollected = true;

	Mesh->SetVisibility(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

