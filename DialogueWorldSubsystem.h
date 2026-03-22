#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DialogueStruct.h"
#include "DialogueWorldSubsystem.generated.h"

class UDataTable;

UCLASS()
class SINGULARITY_API UDialogueWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// called by DialogueTriggerBox
	UFUNCTION(BlueprintCallable)
	void AssignAndStartDialogue(UDataTable* DialogueDT);

private:
	void BuildDialogueCache();
	bool HasAuthority() const;

	void PlayCurrent();
	void OnDialogueEnd();
	void EndDialogue();

	int32 CurrentRowIndex = 0;
	UDataTable* DialogueDataTable = nullptr;
	FTimerHandle DialogueTimerHandle;

	TArray<FDialogueStruct> OrderedDialogues;

};
