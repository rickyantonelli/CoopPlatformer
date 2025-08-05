// Copyright Ricky Antonelli

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerSessionsSubsystem.generated.h"

// creating a custom delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerCreateDelegate, bool, WasSuccessful); // name of delegate, type of parameter that we will accept, name of the parameter
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerJoinDelegate, bool, WasSuccessful);

/**
 * The MultiplayerSessionsSubsystem class handles:
 * Creating and destroying servers
 * Creating and destroying sessions
 * Joining and leaving sessions
 * Transporting the player to a listen server
 */
UCLASS()
class COOPPLATFORMER_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()


public:
	/** Default constructor for UMultiplayerSessionsSubsystem*/
	UMultiplayerSessionsSubsystem();

	/** Overrides initialize to bind delegates */
	void Initialize(FSubsystemCollectionBase& Collection) override; // these exist in the parent so thats why we use override

	/** Overrides Deinitialize*/
	void Deinitialize() override;

	/** The session interface, that we bind delegates to */
	IOnlineSessionPtr SessionInterface;

	/**
	* Creates the server, sets properties, and destroys the server if needed
	* @param ServerName: The name of the server
	*/
	UFUNCTION(BlueprintCallable)
	void CreateServer(FString ServerName);

	/**
	* Finds the server
	* @param ServerName: The name of the server
	*/
	UFUNCTION(BlueprintCallable)
	void FindServer(FString ServerName);

	/**
	* When the session created delegate has been fired, travels the server
	* @params SessionName: The name of the session
	* @params WasSuccessful: Whether the session was created successfully
	*/
	void OnCreateSessionComplete(FName SessionName, bool WasSuccessful);

	/**
	* When the session destroyed delegate has been fired
	* @params SessionName: The name of the session
	* @params WasSuccessful: Whether the session was created successfully
	*/
	void OnDestroySessionComplete(FName SessionName, bool WasSuccessful);

	/**
	* When the join session delegate has been fired, joins the server
	* @params SessionName: The name of the session
	* @params WasSuccessful: Whether the session was created successfully
	*/
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/**
	* When the find session delegate has been fired, attempts to join the server
	* @params WasSuccessful: Whether the session was created successfully
	*/
	void OnFindSessionsComplete(bool WasSuccessful);

	/** For creating a server after we destroy the last */
	bool CreateServerAfterDestroy;

	/** The name of the server to destroy */
	FString DestroyServerName;

	/** The name of the server to find */
	FString ServerNameToFind;

	/** The session name */
	FName MySessionName;

	/** The FOnlineSessionSearch to look for a session */
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	/** The delegate for creating a server */
	UPROPERTY(BlueprintAssignable)
	FServerCreateDelegate ServerCreateDel;

	/** The delegate for destroying a server */
	UPROPERTY(BlueprintAssignable)
	FServerJoinDelegate ServerJoinDel;

	/** The path to the map that the server should travel to */
	UPROPERTY(BlueprintReadWrite)
	FString GameMapPath;

};

