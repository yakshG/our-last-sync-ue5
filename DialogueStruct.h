#pragma once

#include "CoreMinimal.h"
#include "DialogueStruct.generated.h"

USTRUCT(BlueprintType)
struct FDialogueStruct : public FTableRowBase
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Order = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundWave* Audio;
};