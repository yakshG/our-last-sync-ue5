#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameMode.generated.h"

class ABaseRobot;
class ASPlayerController;

UCLASS()
class SINGULARITY_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Players")
	TArray<TSubclassOf<ABaseRobot>> RobotClasses;

	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<ASPlayerController*> RealPlayers;

public:

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

};
