// Copyright Ricky Antonelli

#include "Mechanics/Visuals/SpriteTrailActor.h"

#include "PaperSprite.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ASpriteTrailActor::ASpriteTrailActor()
{
	PrimaryActorTick.bCanEverTick = true;
	// Sample endpoint positions after everything else has moved this frame.
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;

	// Purely cosmetic, so each machine runs its own trail rather than replicating it.
	bReplicates = false;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);
	RootComp->SetMobility(EComponentMobility::Movable);

	Spacing = 60.0f;
	TravelSpeed = 200.0f;
	TrailColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.25f);
	TrailSpriteScale = 1.0f;
	bAlignToDirection = true;
	FadeInDistance = 40.0f;
	FadeOutDistance = 40.0f;
	bFillLineOnBeginPlay = true;
	MaxSpritesInFlight = 64;
	SortPriority = -1;

	FromOffset = FVector::ZeroVector;
	ToOffset = FVector::ZeroVector;

	// Sprites imported against the masked sprite material clip everything below an alpha of
	// 0.333, so force a translucent material to keep faint trails visible for any source sprite.
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TranslucentSpriteMaterial(TEXT("/Paper2D/TranslucentUnlitSpriteMaterial.TranslucentUnlitSpriteMaterial"));
	if (TranslucentSpriteMaterial.Succeeded())
	{
		TrailMaterial = TranslucentSpriteMaterial.Object;
	}

	bTrailActive = true;
	EmitAccumulator = 0.0f;
}

void ASpriteTrailActor::BeginPlay()
{
	Super::BeginPlay();

	if (!bFillLineOnBeginPlay)
	{
		return;
	}

	FVector Start;
	FVector Direction;
	float Length = 0.0f;
	if (!bTrailActive || !TrailSprite || !ResolveLine(Start, Direction, Length))
	{
		return;
	}

	const FRotator Rotation = bAlignToDirection ? FRotationMatrix::MakeFromX(Direction).Rotator() : GetActorRotation();
	const float Step = FMath::Max(Spacing, 1.0f);

	for (float Distance = 0.0f; Distance < Length && ActiveDashes.Num() < MaxSpritesInFlight; Distance += Step)
	{
		EmitDash(Distance, Start, Direction, Rotation, Length);
	}
}

void ASpriteTrailActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector Start;
	FVector Direction;
	float Length = 0.0f;

	if (!bTrailActive || !TrailSprite || !ResolveLine(Start, Direction, Length))
	{
		RetireAllDashes();
		return;
	}

	const FRotator Rotation = bAlignToDirection ? FRotationMatrix::MakeFromX(Direction).Rotator() : GetActorRotation();

	// Advance everything in flight, removing any sprite that has reached the destination.
	for (int32 Index = ActiveDashes.Num() - 1; Index >= 0; --Index)
	{
		FSpriteTrailDash& Dash = ActiveDashes[Index];
		Dash.Distance += TravelSpeed * DeltaSeconds;

		if (Dash.Distance >= Length)
		{
			RetireDash(Index);
			continue;
		}

		UpdateDash(Dash, Start, Direction, Rotation, Length);
	}

	// Emit new sprites at a cadence that keeps the on-screen gap equal to Spacing.
	if (TravelSpeed <= 0.0f)
	{
		return;
	}

	const float EmitInterval = FMath::Max(Spacing, 1.0f) / TravelSpeed;
	EmitAccumulator += DeltaSeconds;

	while (EmitAccumulator >= EmitInterval)
	{
		EmitAccumulator -= EmitInterval;

		if (ActiveDashes.Num() >= MaxSpritesInFlight)
		{
			EmitAccumulator = 0.0f;
			break;
		}

		// Offset by the leftover time so spacing stays even regardless of frame rate.
		const float StartDistance = EmitAccumulator * TravelSpeed;
		if (StartDistance < Length)
		{
			EmitDash(StartDistance, Start, Direction, Rotation, Length);
		}
	}
}

bool ASpriteTrailActor::ResolveLine(FVector& OutStart, FVector& OutDirection, float& OutLength) const
{
	if (!FromActor || !ToActor)
	{
		return false;
	}

	OutStart = GetEndpointLocation(FromActor, FromOffset);
	const FVector End = GetEndpointLocation(ToActor, ToOffset);

	const FVector Delta = End - OutStart;
	OutLength = Delta.Size();

	if (OutLength <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	OutDirection = Delta / OutLength;
	return true;
}

FVector ASpriteTrailActor::GetEndpointLocation(const AActor* Endpoint, const FVector& LocalOffset) const
{
	if (!Endpoint)
	{
		return GetActorLocation();
	}

	if (LocalOffset.IsNearlyZero())
	{
		return Endpoint->GetActorLocation();
	}

	return Endpoint->GetActorTransform().TransformPosition(LocalOffset);
}

void ASpriteTrailActor::EmitDash(float StartDistance, const FVector& Start, const FVector& Direction, const FRotator& Rotation, float Length)
{
	UPaperSpriteComponent* Component = AcquireDashComponent();
	if (!Component)
	{
		return;
	}

	Component->SetSprite(TrailSprite);

	// Applied after SetSprite, since the slot count comes from the sprite itself.
	if (TrailMaterial)
	{
		const int32 NumMaterials = FMath::Max(Component->GetNumMaterials(), 1);
		for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
		{
			if (Component->GetMaterial(MaterialIndex) != TrailMaterial)
			{
				Component->SetMaterial(MaterialIndex, TrailMaterial);
			}
		}
	}

	Component->SetVisibility(true);

	FSpriteTrailDash Dash;
	Dash.Component = Component;
	Dash.Distance = StartDistance;

	UpdateDash(Dash, Start, Direction, Rotation, Length);
	ActiveDashes.Add(Dash);
}

void ASpriteTrailActor::UpdateDash(FSpriteTrailDash& Dash, const FVector& Start, const FVector& Direction, const FRotator& Rotation, float Length) const
{
	if (!Dash.Component)
	{
		return;
	}

	Dash.Component->SetWorldLocationAndRotation(Start + Direction * Dash.Distance, Rotation);

	// Fade up on departure and down on approach so arrivals dissolve instead of popping.
	float Alpha = TrailColor.A;
	if (FadeInDistance > 0.0f)
	{
		Alpha *= FMath::Clamp(Dash.Distance / FadeInDistance, 0.0f, 1.0f);
	}
	if (FadeOutDistance > 0.0f)
	{
		Alpha *= FMath::Clamp((Length - Dash.Distance) / FadeOutDistance, 0.0f, 1.0f);
	}

	FLinearColor Color = TrailColor;
	Color.A = Alpha;
	Dash.Component->SetSpriteColor(Color);
}

UPaperSpriteComponent* ASpriteTrailActor::AcquireDashComponent()
{
	// Reuse a retired component when one is available; behaviour is identical to creating a
	// fresh one, it just avoids allocating and registering a component several times a second.
	while (DashPool.Num() > 0)
	{
		UPaperSpriteComponent* Pooled = DashPool.Pop(EAllowShrinking::No).Get();
		if (Pooled)
		{
			return Pooled;
		}
	}

	UPaperSpriteComponent* Component = NewObject<UPaperSpriteComponent>(this);
	if (!Component)
	{
		return nullptr;
	}

	Component->SetMobility(EComponentMobility::Movable);
	Component->SetupAttachment(RootComp);
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component->SetGenerateOverlapEvents(false);
	Component->SetCastShadow(false);
	Component->SetTranslucentSortPriority(SortPriority);
	Component->SetWorldScale3D(FVector(TrailSpriteScale));
	Component->RegisterComponent();

	return Component;
}

void ASpriteTrailActor::RetireDash(int32 Index)
{
	if (!ActiveDashes.IsValidIndex(Index))
	{
		return;
	}

	if (UPaperSpriteComponent* Component = ActiveDashes[Index].Component.Get())
	{
		Component->SetVisibility(false);
		DashPool.Add(Component);
	}

	ActiveDashes.RemoveAtSwap(Index, 1, EAllowShrinking::No);
}

void ASpriteTrailActor::RetireAllDashes()
{
	for (int32 Index = ActiveDashes.Num() - 1; Index >= 0; --Index)
	{
		RetireDash(Index);
	}

	EmitAccumulator = 0.0f;
}

void ASpriteTrailActor::SetEndpoints(AActor* NewFromActor, AActor* NewToActor)
{
	FromActor = NewFromActor;
	ToActor = NewToActor;
	RetireAllDashes();
}

void ASpriteTrailActor::SetTrailSprite(UPaperSprite* NewSprite)
{
	TrailSprite = NewSprite;

	for (FSpriteTrailDash& Dash : ActiveDashes)
	{
		if (Dash.Component)
		{
			Dash.Component->SetSprite(TrailSprite);
		}
	}
}

void ASpriteTrailActor::SetTrailActive(bool bNewActive)
{
	if (bTrailActive == bNewActive)
	{
		return;
	}

	bTrailActive = bNewActive;

	if (!bTrailActive)
	{
		RetireAllDashes();
	}
}
