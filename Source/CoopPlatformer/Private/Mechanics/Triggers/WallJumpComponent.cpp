// Copyright Ricky Antonelli


#include "Mechanics/Triggers/WallJumpComponent.h"
#include <Kismet/GameplayStatics.h>
#include "Components/BoxComponent.h"
#include "Character/MyPaperCharacter.h"
#include <PaperTileMapComponent.h>

UWallJumpComponent::UWallJumpComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UWallJumpComponent::BeginPlay()
{
	Super::BeginPlay();

	UBoxComponent* Box = GetOwner()->FindComponentByClass<UBoxComponent>();

	if (Box)
	{
		Box->OnComponentHit.AddDynamic(this, &UWallJumpComponent::OnBoxComponentHit);
	}

	UPaperTileMapComponent* TileMap = GetOwner()->FindComponentByClass<UPaperTileMapComponent>();
	if (TileMap)
	{
		TileMap->OnComponentHit.AddDynamic(this, &UWallJumpComponent::OnTilemapComponentHit);
	}
}

void UWallJumpComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWallJumpComponent::OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor->ActorHasTag("Player")) return;

	AMyPaperCharacter* HittingCharacter = Cast<AMyPaperCharacter>(OtherActor);

	if (!HittingCharacter) return;
	if (HittingCharacter->bInWallJumpTimer)
	{
		return; // already wall jumping
	}

	if (FMath::Abs(HittingCharacter->PreviousVelocity.X) < HittingCharacter->WallJumpMinVelocity)
	{
		// print the velocity, and that its too slow
		return; // only trigger if coming in with some X velocity
	}

	const FVector Right = HitComponent->GetForwardVector();
	const float Dot = FVector::DotProduct(Right, Hit.Normal);

	// only fire off if its the side of a wall
	if (Dot != -1.0f && Dot != 1.0f) return;

	// ensure we are not to close to end of the box's bounds

	const bool bIsLeft = Dot <= 0.9f;

	HittingCharacter->OnWallHit(bIsLeft);
}

void UWallJumpComponent::OnTilemapComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor->ActorHasTag("Player")) return;

	AMyPaperCharacter* HittingCharacter = Cast<AMyPaperCharacter>(OtherActor);

	if (!HittingCharacter) return;
	if (HittingCharacter->bInWallJumpTimer)
	{
		return; // already wall jumping
	}

	if (FMath::Abs(HittingCharacter->PreviousVelocity.X) < HittingCharacter->WallJumpMinVelocity)
	{
		// print the velocity, and that its too slow
		return; // only trigger if coming in with some X velocity
	}

	const FVector Right = HitComponent->GetForwardVector();
	const float Dot = FVector::DotProduct(Right, Hit.Normal);

	// only fire off if its the side of a wall
	if (Dot != -1.0f && Dot != 1.0f) return;

	// perform a linetrace to see how close to the ground or ceiling we are
	// if it is closer than HittingCharacter->fWallJumpBounds then not an eligible wall jump

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(HittingCharacter);

	FHitResult MutableHit = Hit;
	const bool bHitGround = GetWorld()->LineTraceSingleByChannel(MutableHit,
		OtherActor->GetActorLocation(), 
		OtherActor->GetActorLocation() - FVector(0.0f, 0.0f, HittingCharacter->WallJumpBounds), 
		ECC_Visibility,
		Params);

	const bool bHitCeiling = GetWorld()->LineTraceSingleByChannel(MutableHit,
		OtherActor->GetActorLocation(),
		OtherActor->GetActorLocation() + FVector(0.0f, 0.0f, HittingCharacter->WallJumpBounds),
		ECC_Visibility,
		Params);

	if (bHitGround || bHitCeiling) {
		return;
	}

	const bool bIsLeft = Dot <= 0.9f;

	HittingCharacter->OnWallHit(bIsLeft);
}
