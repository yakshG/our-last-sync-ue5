#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSubsystem.generated.h"

UCLASS()
class SINGULARITY_API UMultiplayerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FString GameMapPath;

	UPROPERTY(BlueprintReadOnly)
	bool IsSteamClientRunning = false;

protected:
	FString TravelURL;
	bool SessionCreated;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FDelegateHandle CreateSessionCompleteHandle;
	FDelegateHandle DestroySessionCompleteHandle;
	FDelegateHandle JoinSessionCompleteHandle;
	FDelegateHandle InviteAcceptedHandle;

public:
	UMultiplayerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable)
	void CreateSessionInternal();

	UFUNCTION(BlueprintCallable)
	void ShowInviteUI();

	UFUNCTION(BlueprintCallable)
	void TraveltoMap();

protected:
	
	void OnSessionCreated(FName SessionName, bool WasSuccessful);
	void OnSessionDestroyed(FName SessionName, bool WasSuccessful);
	void OnSessionInviteAccepted(bool WasSuccessful, int32 ControllerID, FUniqueNetIdPtr UserID, const FOnlineSessionSearchResult& InviteResult);
	void OnSessionJoined(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	
};
