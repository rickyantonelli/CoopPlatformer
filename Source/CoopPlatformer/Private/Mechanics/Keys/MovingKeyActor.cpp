// Copyright Ricky Antonelli


#include "Mechanics/Keys/MovingKeyActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AMovingKeyActor::AMovingKeyActor()
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

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(RootComp);
	Spline->bDrawDebug = true;

	MoveSpeed = 50.f;
	CooldownTimer = 0.5f;
	SplineDistance = 0.f;

	bActivated = false;
	bOnCooldown = false;
	bMovingForward = false;
}

// Called when the game starts or when spawned
void AMovingKeyActor::BeginPlay()
{
	Super::BeginPlay();
	
	Box->OnComponentBeginOverlap.AddDynamic(this, &AMovingKeyActor::OnBoxCollision);
	Spline->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

// Called every frame
void AMovingKeyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bActivated)
	{
		if (HasAuthority()) // authority because we only want the server to do this
		{
			float SplineLength = Spline->GetSplineLength();
			SplineDistance += bMovingForward ? MoveSpeed * DeltaTime : -MoveSpeed * DeltaTime;

			FVector NewLocation = Spline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);
			SetActorLocation(NewLocation);

			if (bMovingForward && SplineDistance >= SplineLength || !bMovingForward && SplineDistance == 0) bActivated = false;
		}
		else
		{
			FVector NewLocation = Spline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);
			SetActorLocation(NewLocation);
		}
	}
}

void AMovingKeyActor::OnBoxCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->ActorHasTag("Ball")) return;
	if (bOnCooldown) return;
	if (!HasAuthority()) return;

	bActivated = true;
	bOnCooldown = true;
	bMovingForward = !bMovingForward;

	FTimerHandle CooldownHandle;
	GetWorld()->GetTimerManager().SetTimer(CooldownHandle, [&]() { bOnCooldown = false;  }, CooldownTimer, false);
}

void AMovingKeyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMovingKeyActor, bActivated);
	DOREPLIFETIME(AMovingKeyActor, bOnCooldown);
	DOREPLIFETIME(AMovingKeyActor, bMovingForward);
	DOREPLIFETIME(AMovingKeyActor, StartPoint);
	DOREPLIFETIME(AMovingKeyActor, EndPoint);
	DOREPLIFETIME(AMovingKeyActor, SplineDistance);
}