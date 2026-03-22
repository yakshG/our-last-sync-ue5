#include "MultiplayerSubsystem.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineExternalUIInterface.h"

UMultiplayerSubsystem::UMultiplayerSubsystem()
{
	GameMapPath = "";
	TravelURL = "";
	SessionCreated = false;
}

void UMultiplayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{

		IsSteamClientRunning = (OnlineSubsystem->GetSubsystemName() == "Steam");
		SessionInterface = OnlineSubsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			CreateSessionCompleteHandle = SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiplayerSubsystem::OnSessionCreated);
			DestroySessionCompleteHandle = SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMultiplayerSubsystem::OnSessionDestroyed);
			JoinSessionCompleteHandle = SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiplayerSubsystem::OnSessionJoined);
			InviteAcceptedHandle = SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UMultiplayerSubsystem::OnSessionInviteAccepted);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[MultiplayerSubsystem] Session interface is invalid."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MultiplayerSubsystem] no online subsystem found."));
	}
}

void UMultiplayerSubsystem::Deinitialize()
{
	if (SessionInterface)
	{
		SessionInterface->OnCreateSessionCompleteDelegates.Remove(CreateSessionCompleteHandle);
		SessionInterface->OnDestroySessionCompleteDelegates.Remove(DestroySessionCompleteHandle);
		SessionInterface->OnJoinSessionCompleteDelegates.Remove(JoinSessionCompleteHandle);
		SessionInterface->OnSessionUserInviteAcceptedDelegates.Remove(InviteAcceptedHandle);
	}
}

void UMultiplayerSubsystem::CreateSessionInternal()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[MultiplayerSubsystem] CreateSession: session interface invalid."));
		return;
	}

	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);

	if (ExistingSession)
	{
		UE_LOG(LogTemp, Log, TEXT("[MultiplayerSubsystem] Existing session found. Destroying before creating new one."));
		SessionCreated = false;
		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsDedicated = false;
	SessionSettings.NumPublicConnections = 2;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;

	UE_LOG(LogTemp, Log, TEXT("[MultiplayerSubsystem] Creating session."));
	
	bool Success = SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);

	if (!Success)
	{
		UE_LOG(LogTemp, Error, TEXT("[MultiplayerSubsystem] CreateSession returned false."));
	}
}

void UMultiplayerSubsystem::OnSessionCreated(FName SessionName, bool WasSuccessful)
{
	if (!WasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("[MultiplayerSubsystem] Session creation failed."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[MultiplayerSubsystem] Session created: %s"), *SessionName.ToString());

	SessionCreated = true;

	TravelURL = GameMapPath + "?listen";
	
	UE_LOG(LogTemp, Warning, TEXT("[MultiplayerSubsystem] TravelURL: %s"), *TravelURL);
	
}

void UMultiplayerSubsystem::OnSessionDestroyed(FName SessionName, bool WasSuccessful)
{
	if (!WasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("[MultiplayerSubsystem] Failed to destroy existing session '%s'."), *SessionName.ToString());
		return;
	}

	if (SessionCreated)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[MultiplayerSubsystem] Session destroyed. Creating a new session."));

	CreateSessionInternal();
}


void UMultiplayerSubsystem::TraveltoMap()
{
	if (SessionCreated)
	{
		GetWorld()->ServerTravel(TravelURL);
	}
}

void UMultiplayerSubsystem::ShowInviteUI()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[MultiplayerSubsystem] ShowInviteUI: no online subsystem."));
		return;
	}

	IOnlineExternalUIPtr ExternalUI = OnlineSubsystem->GetExternalUIInterface();
	if (!ExternalUI.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[MultiplayerSubsystem] ShowInviteUI: external UI interface invalid."));
		return;
	}

	bool Shown = ExternalUI->ShowInviteUI(0, NAME_GameSession);
	if (!Shown)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MultiplayerSubsystem] ShowInviteUI: failed to show Steam invite overlay."));
	}

}

void UMultiplayerSubsystem::OnSessionInviteAccepted(bool WasSuccessful, int32 ControllerID, FUniqueNetIdPtr UserID, const FOnlineSessionSearchResult& InviteResult)
{
	if (!WasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MultiplayerSubsystem] Invite not accepted or failed."));
		return;
	}

	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[MultiplayerSubsystem] OnSessionInviteAccepted: session interface invalid."));
		return;
	}

	FOnlineSessionSearchResult MutableResult = InviteResult;
	MutableResult.Session.SessionSettings.bUsesPresence = true;
	MutableResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;

	bool JoinStarted = SessionInterface->JoinSession(0, NAME_GameSession, MutableResult);
	if (!JoinStarted)
	{
		UE_LOG(LogTemp, Error, TEXT("[MultiplayerSubsystem] JoinSession via invite returned false immediately."));
	}
}

void UMultiplayerSubsystem::OnSessionJoined(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString ConnectString;
		if (SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectString))
		{
			UE_LOG(LogTemp, Log, TEXT("[MultiplayerSubsystem] Joined Session '%s'. Travelling to: %s"), *SessionName.ToString(), *ConnectString);

			APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();

			if (PC)
			{
				PC->ClientTravel(ConnectString, TRAVEL_Absolute);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[MultiplayerSubsystem] Joined session but failed to resolve connect string."));
		}

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MultiplayerSubsystem] JoinSession failed with result: % d"), int32(Result));
	}
}