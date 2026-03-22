#include "SGameState.h"
#include "Net/UnrealNetwork.h"

ASGameState::ASGameState()
{
	bReplicates = true;
}

void ASGameState::SetActiveDialogue(FDialogueStruct Dialogue)
{
	if (!HasAuthority())
		return;

	ActiveDialogue = Dialogue;
	bDialogueActive = true;

	OnDialogueStateChanged.Broadcast();
}

void ASGameState::ClearDialogue()
{
	if (!HasAuthority())
		return;

	bDialogueActive = false;

	OnDialogueStateChanged.Broadcast();
}

void ASGameState::OnRep_DialogueState()
{
	OnDialogueStateChanged.Broadcast();
}

void ASGameState::AddStoredActor(AActor* Actor)
{
	if (!HasAuthority() || !Actor)
		return;

	StoredActors.Add(Actor);
	UE_LOG(LogTemp, Warning, TEXT("[SGameState] AddStoredActor: stored %s. Total: %d"), *Actor->GetName(), StoredActors.Num());
}

bool ASGameState::SpawnConsumedActor(AActor*& OutActor)
{
	if (!HasAuthority() || StoredActors.IsEmpty())
		return false;

	OutActor = StoredActors[0];
	StoredActors.RemoveAt(0);

	UE_LOG(LogTemp, Warning, TEXT("[SGameState] ConsumeOneActor: consumed %s. Remaining: %d"), *OutActor->GetName(), StoredActors.Num());
	return true;
}

bool ASGameState::HasStoredActors() const
{
	return !StoredActors.IsEmpty();
}


void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGameState, bDialogueActive);
	DOREPLIFETIME(ASGameState, ActiveDialogue);
}