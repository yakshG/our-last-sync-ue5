#include "CoreGameInstance.h"

UCoreGameInstance::UCoreGameInstance()
{
	CurrentMode = EMultiplayerMode::None;
}

void UCoreGameInstance::SetLocalGame(const FString& MapName)
{
	CurrentMode = EMultiplayerMode::Local;

	// travel as listen server so GameMode runs server-sdie even for local play

	GetWorld()->ServerTravel(MapName + "?listen");
}

void UCoreGameInstance::SetOnlineGame()
{
	CurrentMode = EMultiplayerMode::Online;

}
