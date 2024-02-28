// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"


void PrintString(const FString& Str)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, Str);
	}
}

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
{
	CreateServerAfterDestroy = false;
	DestroyServerName = "";
	ServerNameToFind = "";
	MySessionName = FName("Co-Op Adventure Session Name:");
}

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// PrintString("MSS Initialize");
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get(); // this is going to return the online subsystem for us (i.e. Steam)
	if (OnlineSubsystem)
	{
		//FString SubsystemName = OnlineSubsystem->GetSubsystemName().ToString(); // returns the name of the subsystem (obviously) - has to do with the value we set in the defaultengine.ini file
		//PrintString(SubsystemName);

		SessionInterface = OnlineSubsystem->GetSessionInterface(); // returns a pointer to the session interface of type IOnlineSessionPointer
		if (SessionInterface.IsValid()) // checks if this is valid or not since this is a shared pointer
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete); // if you put an '&' before a function name, it will give you the address to that function
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
	PrintString("CreateServer");

	if (ServerName.IsEmpty())
	{
		PrintString("Server name cannot be empty!");
		ServerCreateDel.Broadcast(false); // this is how you fire the delegate, once you do this all the functions that are bound to this delegate are called
		return;
	}

	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(MySessionName);
	if (ExistingSession)
	{
		FString Msg = FString::Printf(TEXT("Session with name %s already exists, destroying it."), *MySessionName.ToString()); // asterisk turns it into a cstyle string
		PrintString(Msg);
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
	PrintString("FindServer");

	if (ServerName.IsEmpty())
	{
		PrintString("ServerName cannot be empty!");
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
	PrintString(FString::Printf(TEXT("OnCreateSessionComplete: %d"), WasSuccessful));

	ServerCreateDel.Broadcast(WasSuccessful);

	if (WasSuccessful)
	{
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
	FString Msg = FString::Printf(TEXT("OnDestroySessionComplete, SessionName: %s, Success: %d"), *SessionName.ToString(), WasSuccessful); // reminder that the asterisk turns it into a cstyle string
	PrintString(Msg);

	if (CreateServerAfterDestroy)
	{
		CreateServerAfterDestroy = false;
		CreateServer(DestroyServerName);
	}
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool WasSuccessful)
{
	if (!WasSuccessful) return;
	if (ServerNameToFind.IsEmpty()) return;
	TArray<FOnlineSessionSearchResult> Results = SessionSearch->SearchResults; // this is an Array of FOnlineSearchResults
	FOnlineSessionSearchResult* CorrectResult = 0; // 0 by default so that it is invalid


	if (Results.Num() > 0)
	{
		FString Msg = FString::Printf(TEXT("%d sessions found."), Results.Num());
		PrintString(Msg);
		for (FOnlineSessionSearchResult Result : Results) // loops over each element in the results array
		{
			if (Result.IsValid())
			{
				FString ServerName = "No-name";
				Result.Session.SessionSettings.Get(FName("SERVER_NAME"), ServerName);

				if (ServerName.Equals(ServerNameToFind)) // fstring has a function called equals
				{
					CorrectResult = &Result; // need the address because correctresult is a pointer
					FString Msg2 = FString::Printf(TEXT("Found server with name = %s"), *ServerName); //Fstrings need to be cstyle strings
					PrintString(Msg2);
					break; // found the one we are looking for, so dont need to keep going
				}
			}
		}

		if (CorrectResult)
		{
			SessionInterface->JoinSession(0, MySessionName, *CorrectResult); // need to dereference the pointer
		}
		else
		{
			PrintString(FString::Printf(TEXT("Couldnt find server with name = %s"), *ServerNameToFind));
			ServerNameToFind = "";
			ServerJoinDel.Broadcast(false);
		}
	}
	else
	{
		PrintString("Zero sessions found!");
		ServerJoinDel.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	ServerJoinDel.Broadcast(Result == EOnJoinSessionCompleteResult::Success);

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString Msg = FString::Printf(TEXT("Successfully joined session %s"), *SessionName.ToString());
		PrintString(Msg);

		FString Address = "";
		bool Success = SessionInterface->GetResolvedConnectString(MySessionName, Address);
		if (Success)
		{
			PrintString(FString::Printf(TEXT("Address: %s"), *Address));
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
		else
		{
			PrintString("GetResolvedConnectString returned false!");
		}
	}
	else
	{
		PrintString("OnJoinSessionComplete failed");
	}
}



