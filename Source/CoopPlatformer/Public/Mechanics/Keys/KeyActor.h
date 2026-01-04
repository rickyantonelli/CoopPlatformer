// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Character/BallActor.h"
#include "PaperSpriteComponent.h"
#include "PaperFlipBookComponent.h"
#include "PaperFlipbook.h"
#include "GameFramework/Actor.h"
#include "KeyActor.generated.h"

class USoundBase;

/** KeyActor is the class that is activated by interaction with the BallActor */
UCLASS()
class COOPPLATFORMER_API AKeyActor : public AActor
{
	GENERATED_BODY()
	
public:
	/** Default constructor for AKeyActor - Sets components, replication, and initializes variables*/
	AKeyActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	/** Override for Tick*/
	virtual void Tick(float DeltaTime) override;

	/** Array of active players - to avoid having to constantly get all actors of class and casting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<AActor*> LockedActors;

	/** The root component of the key actor */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TObjectPtr<USceneComponent> RootComp;

	/** The root component of the key actor */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TArray<UPaperSpriteComponent*> SpriteComps;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperSprite> YellowKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperSprite> RedKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperSprite> GreenKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperFlipbook> UnlockFlipbook;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<USoundBase> CompletedSound;

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastTriggerUnlock();

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastTriggerReset();

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastRedKey();

	/** If the key is locked or not, when unlocked stop checking for overlap */
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	bool Locked;

	UPROPERTY(EditAnywhere)
	bool bCanReset;

	UFUNCTION()
	void OnResetActivated();

	UFUNCTION()
	void OnDoorFlipbookFinished();

	UFUNCTION()
	void OnUnlockFlipbookFinished();
};
