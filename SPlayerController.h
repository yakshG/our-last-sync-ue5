#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerWidget.h"
#include "EnhancedInputSubsystems.h"
#include "SPlayerController.generated.h"

class ABaseRobot;

UCLASS()
class SINGULARITY_API ASPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem;

	virtual void BeginPlay() override;

	void CreatePlayerWidget();
	void HandleDialogueStateChanged();


private:
	UPROPERTY()
	UPlayerWidget* PlayerWidget;

	int32 ControlledPawnID;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	TSubclassOf<ABaseRobot> SelectedRobotClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	ABaseRobot* PlayerPawn;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Player")
	int32 SelectedRobotIndex = 0;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
