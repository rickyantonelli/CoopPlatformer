// Copyright Ricky Antonelli
// Code written by Ricky Antonelli, but used in reference to https://www.udemy.com/course/unreal-engine-5-multiplayer 

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
{
	CreateServerAfterDestroy = false;
	DestroyServerName = "";
	ServerNameToFind = "";
	MySessionName = FName("Nova Session Name:");
}

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get(); // this is going to return the online subsystem for us (i.e. Steam)
	if (OnlineSubsystem)
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface(); // returns a pointer to the session interface of type IOnlineSessionPointer
		if (SessionInterface.IsValid()) // checks if this is valid or not since this is a shared pointer
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionComplete);
		}
	}
}

void UMultiplayerSessionsSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Warning, TEXT("MSS Deinitialize"));
}

void UMultiplayerSessionsSubsystem::CreateServer(FString ServerName)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, IOnlineSubsystem::Get()->GetSubsystemName().ToString());
	if (ServerName.IsEmpty())
	{
		// do nothing if we have an empty server name
		ServerCreateDel.Broadcast(false);
		return;
	}

	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(MySessionName);
	if (ExistingSession)
	{
		CreateServerAfterDestroy = true;
		DestroyServerName = ServerName;
		SessionInterface->DestroySession(MySessionName); // want to destroy the session if it exists so we can create a new one, but we need to wait for the delegate to fire
		return; // since we have to wait, its better off just not doing anything else (for now) - aka we will have to click create server twice
	}

	FOnlineSessionSettings SessionSettings;
	bool IsLan = false;

	SessionSettings.bAllowJoinInProgress = true; // allows players to join while the game is in progress
	SessionSettings.bIsDedicated = false; // whether or not the server is a dedicated server
	SessionSettings.bShouldAdvertise = true; // should this be publicly advertised on the online servers
	SessionSettings.NumPublicConnections = 2; // how many players we want in a game session - maximum
	SessionSettings.bUseLobbiesIfAvailable = true; // use lobbies API if it's available
	SessionSettings.bUsesPresence = true; // something Steam uses, has to do with player info and regions
	SessionSettings.bAllowJoinViaPresence = true; // another presence thing for steam
	SessionSettings.bIsLANMatch = IsLan; // depends on the subsystem, want this to be true for NULL but false for Steam



	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		IsLan = true;
	}

	SessionSettings.Set(FName("SERVER_NAME"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing); // we can store this into session settings, so that we can get this back later


	SessionInterface->CreateSession(0, MySessionName, SessionSettings);
	// the session is not created until we hear back from Steam
}

void UMultiplayerSessionsSubsystem::FindServer(FString ServerName)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, IOnlineSubsystem::Get()->GetSubsystemName().ToString());
	if (ServerName.IsEmpty())
	{
		// do nothing if we have an empty server name
		ServerJoinDel.Broadcast(false);
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	bool IsLan = false;
	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		IsLan = true;
	}

	SessionSearch->bIsLanQuery = IsLan; // will make the search purely a lan search
	SessionSearch->MaxSearchResults = 9999; // the max search results that we can get
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals); // this is an object called query settings that we can set - only search for sessions with presence

	ServerNameToFind = ServerName;

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef()); // will turn the TSharedRef to a shared pointer
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool WasSuccessful)
{
	// session was successfully created - so lets do some stuff
	ServerCreateDel.Broadcast(WasSuccessful);

	if (WasSuccessful)
	{
		// defaults to the third person map if we can't load the map we want - should probably just make this a check()
		FString Path = "/Game/ThirdPerson/Maps/ThirdPersonMap?listen";

		if (!GameMapPath.IsEmpty())
		{
			Path = FString::Printf(TEXT("%s?listen"), *GameMapPath);
		}

		GetWorld()->ServerTravel(Path); // moves the server to a new level - `?listen` means as a listen server
	}
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool WasSuccessful)
{
	// session was successfully destroyed
	if (CreateServerAfterDestroy)
	{
		CreateServerAfterDestroy = false;
		CreateServer(DestroyServerName);
	}
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool WasSuccessful)
{
	// successfully finished searching, but didnt necessarily find something to join
	if (!WasSuccessful) return;
	if (ServerNameToFind.IsEmpty()) return;

	TArray<FOnlineSessionSearchResult> Results = SessionSearch->SearchResults;
	FOnlineSessionSearchResult* CorrectResult = 0; // 0 by default so that it is invalid

	if (Results.Num() > 0)
	{
		for (FOnlineSessionSearchResult Result : Results)
		{
			if (Result.IsValid())
			{
				FString ServerName = "No-name";
				Result.Session.SessionSettings.Get(FName("SERVER_NAME"), ServerName);

				if (ServerName.Equals(ServerNameToFind))
				{
					CorrectResult = &Result;
					break; // found the one we are looking for, so dont need to keep going
				}
			}
		}

		if (CorrectResult)
		{
			SessionInterface->JoinSession(0, MySessionName, *CorrectResult);
		}
		else
		{
			// we are not joining the server
			ServerNameToFind = "";
			ServerJoinDel.Broadcast(false);
		}
	}
	else
	{
		// we are not joining the server
		ServerJoinDel.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	ServerJoinDel.Broadcast(Result == EOnJoinSessionCompleteResult::Success);

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString Address = "";
		bool Success = SessionInterface->GetResolvedConnectString(MySessionName, Address);
		if (Success)
		{
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}



