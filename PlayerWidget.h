#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueStruct.h"
#include "BaseRobot.h"
#include "PlayerWidget.generated.h"

UCLASS()
class SINGULARITY_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowDialogue(FDialogueStruct ActiveDialogue);

	UFUNCTION(BlueprintImplementableEvent)
	void HideDialogue();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 PlayerID;
};
