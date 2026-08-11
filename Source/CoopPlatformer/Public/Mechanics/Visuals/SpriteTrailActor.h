// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperSpriteComponent.h"
#include "SpriteTrailActor.generated.h"

class UMaterialInterface;
class UPaperSprite;

/** A single sprite currently travelling along the trail. */
USTRUCT()
struct FSpriteTrailDash
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UPaperSpriteComponent> Component = nullptr;

	/** How far along the line this sprite has travelled, in cm. */
	float Distance = 0.0f;
};

/**
 * Emits a repeating sprite (a dash, dot, arrow, anything) that travels from one actor to another
 * and is removed the moment it arrives, faintly indicating that the two actors are linked.
 *
 * The trail is length-agnostic: the number of sprites in flight is derived from the live distance
 * between the endpoints, so the endpoints can be moved freely at runtime or in the editor.
 */
UCLASS()
class COOPPLATFORMER_API ASpriteTrailActor : public AActor
{
	GENERATED_BODY()

public:
	ASpriteTrailActor();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	/** The sprite repeated along the line. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sprite Trail")
	TObjectPtr<UPaperSprite> TrailSprite;

	/** Actor the sprites travel away from. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sprite Trail")
	TObjectPtr<AActor> FromActor;

	/** Actor the sprites travel towards, and are removed at. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sprite Trail")
	TObjectPtr<AActor> ToActor;

	/** Offset from FromActor's origin, in its local space. Lets both endpoints live on a single actor. */
	UPROPERTY(EditAnywhere, Category = "Sprite Trail")
	FVector FromOffset;

	/** Offset from ToActor's origin, in its local space. */
	UPROPERTY(EditAnywhere, Category = "Sprite Trail")
	FVector ToOffset;

	/** Gap between consecutive sprites, in cm. Also drives how often a new one is emitted. */
	UPROPERTY(EditAnywhere, Category = "Sprite Trail", meta = (ClampMin = "1.0"))
	float Spacing;

	/** Travel speed along the line, in cm/s. */
	UPROPERTY(EditAnywhere, Category = "Sprite Trail", meta = (ClampMin = "0.0"))
	float TravelSpeed;

	/** Tint applied to every sprite. The alpha sets the overall faintness. */
	UPROPERTY(EditAnywhere, Category = "Sprite Trail")
	FLinearColor TrailColor;

	/** Uniform scale applied to every sprite. */
	UPROPERTY(EditAnywhere, Category = "Sprite Trail", meta = (ClampMin = "0.01"))
	float TrailSpriteScale;

	/** Rotate each sprite to face along the line. Leave off for sprites that should stay upright. */
	UPROPERTY(EditAnywhere, Category = "Sprite Trail")
	bool bAlignToDirection;

	/** Distance over which a sprite fades up after leaving FromActor, in cm. 0 disables. */
	UPROPERTY(EditAnywhere, Category = "Sprite Trail", meta = (ClampMin = "0.0"))
	float FadeInDistance;

	/** Distance over which a sprite fades out as it nears ToActor, in cm. 0 disables. */
	UPROPERTY(EditAnywhere, Category = "Sprite Trail", meta = (ClampMin = "0.0"))
	float FadeOutDistance;

	/** Fill the whole line on BeginPlay instead of populating it one sprite at a time. */
	UPROPERTY(EditAnywhere, Category = "Sprite Trail")
	bool bFillLineOnBeginPlay;

	/** Safety cap on sprites in flight at once. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Sprite Trail", meta = (ClampMin = "1"))
	int32 MaxSpritesInFlight;

	/** Sort priority for the translucent sprites, so the trail can be pushed behind gameplay art. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Sprite Trail")
	int32 SortPriority;

	/**
	 * Material every sprite in the trail is drawn with. Defaults to Paper2D's translucent unlit
	 * sprite material, because a sprite authored with the masked material clips any pixel whose
	 * alpha is below 0.333 and would therefore render nothing at faint TrailColor alphas.
	 * Clear this to fall back to each sprite's own material.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Sprite Trail")
	TObjectPtr<UMaterialInterface> TrailMaterial;

	/** Assign both endpoints at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Sprite Trail")
	void SetEndpoints(AActor* NewFromActor, AActor* NewToActor);

	/** Swap the repeated sprite at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Sprite Trail")
	void SetTrailSprite(UPaperSprite* NewSprite);

	/** Start or stop emitting. Stopping clears everything currently in flight. */
	UFUNCTION(BlueprintCallable, Category = "Sprite Trail")
	void SetTrailActive(bool bNewActive);

private:
	/** Sprites currently travelling. */
	UPROPERTY()
	TArray<FSpriteTrailDash> ActiveDashes;

	/** Retired components, kept for reuse so arrivals don't churn allocations. */
	UPROPERTY()
	TArray<TObjectPtr<UPaperSpriteComponent>> DashPool;

	bool bTrailActive;
	float EmitAccumulator;

	bool ResolveLine(FVector& OutStart, FVector& OutDirection, float& OutLength) const;
	FVector GetEndpointLocation(const AActor* Endpoint, const FVector& LocalOffset) const;

	void EmitDash(float StartDistance, const FVector& Start, const FVector& Direction, const FRotator& Rotation, float Length);
	void UpdateDash(FSpriteTrailDash& Dash, const FVector& Start, const FVector& Direction, const FRotator& Rotation, float Length) const;
	void RetireDash(int32 Index);
	void RetireAllDashes();

	UPaperSpriteComponent* AcquireDashComponent();
};
