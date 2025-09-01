// Copyright Ricky Antonelli


#include "Mechanics/Movement/FreezeToken.h"

// Sets default values
AFreezeToken::AFreezeToken()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComp);
	Box->SetIsReplicated(true);
	Box->SetCollisionProfileName(FName("OverlapAllDynamic"));

	Sprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	Sprite->SetupAttachment(Box);
	Sprite->SetCollisionProfileName(FName("NoCollision"));

	RespawnTimer = 10.0f;

	CanBeCollected = true;

	Tags.Add(FName("Freeze"));
}

// Called when the game starts or when spawned
void AFreezeToken::BeginPlay()
{
	Super::BeginPlay();

	Box->OnComponentBeginOverlap.AddDynamic(this, &AFreezeToken::OnBoxCollision);
	
}

// Called every frame
void AFreezeToken::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFreezeToken::CollectToken()
{
	if (CanBeCollected)
	{
		CanBeCollected = false;
		Sprite->SetVisibility(false);
		Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);


		// after the timer, allow to be collected again
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, [&]() {SetCollectable(); }, RespawnTimer, false);
	}
}

void AFreezeToken::SetCollectable()
{
	CanBeCollected = true;

	Sprite->SetVisibility(true);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AFreezeToken::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Dash prototype - holding off for now
	if (OtherActor->ActorHasTag("Player"))
	{
		AMyPaperCharacter* PlayerCharacterActor = Cast<AMyPaperCharacter>(OtherActor);
		PlayerCharacterActor->ApplyFreezeToken();

		CollectToken();

	}
}