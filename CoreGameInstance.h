#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CoreGameInstance.generated.h"

UENUM(BlueprintType)
enum class EMultiplayerMode : uint8
{
	None,
	Local,
	Online
};

UCLASS()
class SINGULARITY_API UCoreGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UCoreGameInstance();
	
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void SetLocalGame(const FString& MapName);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void SetOnlineGame();

	EMultiplayerMode CurrentMode = EMultiplayerMode::Online;
};
