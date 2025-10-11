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
}

// Called when the game starts or when spawned
void AWindActor::BeginPlay()
{
	Super::BeginPlay();

	OriginalExtend = Box->GetUnscaledBoxExtent();
	OriginalScaledExtend = Box->GetScaledBoxExtent();
	OriginalLocation = Box->GetComponentLocation();
	StartLocation = Box->GetComponentLocation();
	UE_LOG(LogTemp, Warning, TEXT("OriginalExtend Location: %s"), *OriginalExtend.ToString());

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

	// if (HasAuthority()) UE_LOG(LogTemp, Warning, TEXT("CurrentExtent: %s"), *Box->GetScaledBoxExtent().ToString());

	if (HasAuthority() && GoAgain)
	{
		FVector Up = GetActorUpVector();
		FVector EndLocation = FVector(Box->GetComponentLocation().X, Box->GetComponentLocation().Y, Box->GetComponentLocation().Z + Box->GetScaledBoxExtent().Z);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.bTraceComplex = true;

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

		bool bHit = GetWorld()->LineTraceSingleByObjectType(Hit, Box->GetComponentLocation(), EndLocation, ObjectParams, Params);

		if (bHit)
		{
			float NewExtentZ = (Hit.Location.Z - (Box->GetComponentLocation().Z - Box->GetScaledBoxExtent().Z)) * 0.5f;

			FVector NewLocation(Box->GetComponentLocation().X, Box->GetComponentLocation().Y, Box->GetComponentLocation().Z - (NewExtentZ / 3));
			FVector NewExtent = FVector(OriginalExtend.X, OriginalExtend.Y, NewExtentZ / Box->GetComponentScale().Z);

			UE_LOG(LogTemp, Warning, TEXT("NewExtent: %s"), *NewExtent.ToString());
			UE_LOG(LogTemp, Warning, TEXT("NewLocation: %s"), *NewLocation.ToString());

			Box->SetBoxExtent(NewExtent);
			Box->SetWorldLocation(NewLocation);

			GoAgain = false;
		}
		//else
		//{
		//	Box->SetBoxExtent(OriginalExtend);
		//	Box->SetWorldLocation(OriginalLocation);
		//}
	}

	if (HasAuthority() && bPushing && OverlappingBall)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pushing Ball Upwards"));
		FVector BallLocation = OverlappingBall->GetActorLocation();
		BallLocation.Z += PushForce * DeltaTime;
		OverlappingBall->SetActorLocation(BallLocation);
	}

}

void AWindActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->ActorHasTag("Ball")) return;
	if (!OverlappingBall || !OverlappingBall->IsMoving) return;
	bPushing = true;

}

void AWindActor::OnBoxCollisionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor->ActorHasTag("Ball")) return;
	bPushing = false;
}