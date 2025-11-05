// Copyright Ricky Antonelli


#include "Mechanics/Platforms/HolderEntryActor.h"
#include "Controller/Controller2D.h"

// Sets default values
AHolderEntryActor::AHolderEntryActor()
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

}

// Called when the game starts or when spawned
void AHolderEntryActor::BeginPlay()
{
	Super::BeginPlay();
	
	Box->OnComponentBeginOverlap.AddDynamic(this, &AHolderEntryActor::OnBoxCollisionEnter);

	Box->OnComponentEndOverlap.AddDynamic(this, &AHolderEntryActor::OnBoxCollisionExit);
}

// Called every frame
void AHolderEntryActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHolderEntryActor::OnBoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->ActorHasTag("Player")) return;
	if (!HasAuthority()) return;

	AMyPaperCharacter* PlayerCharacter = Cast<AMyPaperCharacter>(OtherActor);
	if (!PlayerCharacter) return;

	FVector KickOffVelocity;
	float KickOffHorizontal = 100.f;

	PlayerCharacter->bPassingThrough = false;

	// must be holding ball to pass
	if (bHolderEntry)
	{
		if (!PlayerCharacter->IsHolding)
		{
			/*KickOffVelocity.X = PlayerCharacter->PreviousVelocity.X < 0 ? KickOffHorizontal : -KickOffHorizontal;
			PlayerCharacter->LaunchCharacter(KickOffVelocity, true, true);*/
			APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
			AController2D* C2D = Cast<AController2D>(PC);
			C2D->CollectPlayerFullDeath();
		}
	}
	else
	{
		if (PlayerCharacter->IsHolding)
		{
			/*KickOffVelocity.X = PlayerCharacter->PreviousVelocity.X < 0 ? KickOffHorizontal : -KickOffHorizontal;
			PlayerCharacter->LaunchCharacter(KickOffVelocity, true, true);*/
			APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
			AController2D* C2D = Cast<AController2D>(PC);
			C2D->CollectPlayerFullDeath();
		}
	}
}

void AHolderEntryActor::OnBoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor->ActorHasTag("Player")) return;
	if (!HasAuthority()) return;

	AMyPaperCharacter* PlayerCharacter = Cast<AMyPaperCharacter>(OtherActor);
	if (!PlayerCharacter) return;

	PlayerCharacter->bPassingThrough = true;
}

