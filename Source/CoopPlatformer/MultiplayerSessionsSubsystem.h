// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerSessionsSubsystem.generated.h"

// creating a custom delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerCreateDelegate, bool, WasSuccessful); // name of delegate, type of parameter that we will accept, name of the parameter
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerJoinDelegate, bool, WasSuccessful);
// declaring a delegate type
// dynamic means we are gonna use in blueprint
// multicast means we can bind multiple functions to this
// one param is the argument that the delegate is going to accept


/**
 *
 */
UCLASS()
class COOPPLATFORMER_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()


public:
	UMultiplayerSessionsSubsystem();

	void Initialize(FSubsystemCollectionBase& Collection) override; // these exist in the parent so thats why we use override
	void Deinitialize() override;

	IOnlineSessionPtr SessionInterface;

	UFUNCTION(BlueprintCallable)
	void CreateServer(FString ServerName);

	UFUNCTION(BlueprintCallable)
	void FindServer(FString ServerName);

	void OnCreateSessionComplete(FName SessionName, bool WasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool WasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnFindSessionsComplete(bool WasSuccessful);

	bool CreateServerAfterDestroy;
	FString DestroyServerName;
	FString ServerNameToFind;

	FName MySessionName;

	TSharedPtr<FOnlineSessionSearch> SessionSearch; //TShardPtr is a pointer that is reference counted, so we dont have to deallocate the memory

	UPROPERTY(BlueprintAssignable)
	FServerCreateDelegate ServerCreateDel;

	UPROPERTY(BlueprintAssignable)
	FServerJoinDelegate ServerJoinDel;

	UPROPERTY(BlueprintReadWrite)
	FString GameMapPath;

};

