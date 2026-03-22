#include "DialogueWorldSubsystem.h"
#include "SGameState.h"

void UDialogueWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("[DialogueWorldSubsystem] Initialized in world: %s"), *GetWorld()->GetMapName());
}

bool UDialogueWorldSubsystem::HasAuthority() const
{
	return GetWorld() && GetWorld()->GetAuthGameMode() != nullptr;
}

// builds a cache for the dialogues into a tmap
void UDialogueWorldSubsystem::BuildDialogueCache()
{
	OrderedDialogues.Empty();

	if (!DialogueDataTable)
		return;
	
	const TArray<FName> RowNames = DialogueDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		const FDialogueStruct* Row = DialogueDataTable->FindRow<FDialogueStruct>(RowName, TEXT("DialogueCache"));

		if (!Row)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DialogueWorldSubsystem] Invalid dialogue row: %s"), *RowName.ToString());
			continue;
		}

		OrderedDialogues.Add(*Row);
	}

	OrderedDialogues.Sort([](const FDialogueStruct& A, const FDialogueStruct& B)
		{
			return A.Order < B.Order;
		});

	UE_LOG(LogTemp, Warning, TEXT("[DialogueWorldSubsystem] Built %d ordered dialogue"), OrderedDialogues.Num());
}

// gets the data table
void UDialogueWorldSubsystem::AssignAndStartDialogue(UDataTable* DialogueDT)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DialogueWorldSubsystem] AssignAndStartDialogue called without authority!"));
		return;
	}
	
	if (!DialogueDT)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DialogueWorldSubsystem] Null data table passed!"));
		return;
	}

	DialogueDataTable = DialogueDT;
	BuildDialogueCache();

	CurrentRowIndex = 0;
	PlayCurrent();
}

void UDialogueWorldSubsystem::PlayCurrent()
{
	if (!OrderedDialogues.IsValidIndex(CurrentRowIndex))
	{
		EndDialogue();
		return;
	}

	const FDialogueStruct& Dialogue = OrderedDialogues[CurrentRowIndex];

	ASGameState* GameState = GetWorld()->GetGameState<ASGameState>();
	if (!GameState)
	{
		UE_LOG(LogTemp, Error, TEXT("[DialogueWorldSubsystem] GameState not found."));
		return;
	}

	GameState->SetActiveDialogue(Dialogue);

	GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, this, &UDialogueWorldSubsystem::OnDialogueEnd, Dialogue.Duration, false);

}

void UDialogueWorldSubsystem::OnDialogueEnd()
{	
	CurrentRowIndex++;
	PlayCurrent();
}

void UDialogueWorldSubsystem::EndDialogue()
{
	GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);

	ASGameState* GameState = GetWorld()->GetGameState<ASGameState>();
	if (GameState)
	{
		GameState->ClearDialogue();
	}

	UE_LOG(LogTemp, Warning, TEXT("[DialogueWorldSubsystem] Dialogue ended."))
}

