// Copyright Ricky Antonelli

#include "Mechanics/Triggers/WindActor.h"
#include "Systems/MyGameStateBase.h"

// Sets default values
AWindActor::AWindActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComp);
	Box->SetIsReplicated(true);

	PushForce = 1.0f;
	OverlappingBall = nullptr;
	BlockLocation = FVector::ZeroVector;
}

// Called when the game starts or when spawned
void AWindActor::BeginPlay()
{
	Super::BeginPlay();

	Box->OnComponentBeginOverlap.AddDynamic(this, &AWindActor::OnBoxCollision);
	Box->OnComponentEndOverlap.AddDynamic(this, &AWindActor::OnBoxCollisionEnd);

	AMyGameStateBase* GameState = GetWorld()->GetGameState<AMyGameStateBase>();
	if (GameState && GameState->BallActor)
	{
		OverlappingBall = GameState->BallActor;
	}
	
}

// Called every frame
void AWindActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (OverlappingBall) Params.AddIgnoredActor(OverlappingBall);
	Params.bTraceComplex = true;
	Params.bReturnPhysicalMaterial = false;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FVector BoxExtent;
	FVector Origin;
	GetActorBounds(false, Origin, BoxExtent);

	FVector BottomPoint = Origin - FVector(0.0f, 0.0f, BoxExtent.Z);
	FVector TopPoint = Origin + FVector(0.0f, 0.0f, BoxExtent.Z);


	bool bHit = GetWorld()->LineTraceSingleByObjectType(Hit, BottomPoint, TopPoint, ObjectParams, Params);

	if (bHit)
	{
		if (BlockLocation == FVector::ZeroVector)
		{
			BlockLocation = Hit.Location;
		}
		else if (Hit.Location.Z < BlockLocation.Z)
		{
			BlockLocation = Hit.Location;
		}
		UE_LOG(LogTemp, Warning, TEXT("Wind Blocked at Location: %s"), *BlockLocation.ToString());
	}
	else
	{
		if (BlockLocation != FVector::ZeroVector)
		{
			// validate the blocker is actually gone
			TArray<UPrimitiveComponent*> OverlapComps;
            Box->GetOverlappingComponents(OverlapComps);
			if (OverlapComps.Num() > 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Keeping Blocked at Location: %s"), *BlockLocation.ToString());
			}
			else
			{
				BlockLocation = FVector::ZeroVector;
				UE_LOG(LogTemp, Warning, TEXT("Wind No Longer Blocked"));
			}
		}
		else
		{
			BlockLocation = FVector::ZeroVector;
		}
	}

	if (!bPushing) return;
	if (!OverlappingBall || !OverlappingBall->IsMoving)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Pushing: No Overlapping Ball or Ball Not Moving"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Pushing Ball Upwards"));
	FVector BallLocation = OverlappingBall->GetActorLocation();
	BallLocation.Z += PushForce * DeltaTime;
	OverlappingBall->SetActorLocation(BallLocation);

}

void AWindActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->ActorHasTag("Ball")) return;
	if (!OverlappingBall || !OverlappingBall->IsMoving) return;
	if (!HasAuthority()) return;

	if (BlockLocation != FVector::ZeroVector)
	{
		FVector BallLocation = OverlappingBall->GetActorLocation();
		if (BallLocation.Z > BlockLocation.Z)
		{
			// wind is being blocked at this point, return
			UE_LOG(LogTemp, Warning, TEXT("Wind is Blocked, Not Pushing"));
			return;
		}
	}

	bPushing = true;
}

void AWindActor::OnBoxCollisionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;
	if (!OtherActor->ActorHasTag("Ball")) return;
	bPushing = false;
}