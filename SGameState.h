#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DialogueStruct.h"
#include "SGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnDialogueStateChanged);

UCLASS()
class SINGULARITY_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	ASGameState();
	
	// server-only array; populated by BlackHole, consumed by WhiteHole
	// no replication needed as both systems run authority-only
	TArray<AActor*> StoredActors;

	// dialogue system called by WorldSubsystem
	void SetActiveDialogue(FDialogueStruct Dialogue);
	void ClearDialogue();

	bool IsDialogueActive() const { return bDialogueActive; }
	const FDialogueStruct& GetActiveDialogue() const { return ActiveDialogue; }

	FOnDialogueStateChanged OnDialogueStateChanged;

	void AddStoredActor(AActor* Actor);  // should be called by black hole only
	bool SpawnConsumedActor(AActor*& OutActor);	
	bool HasStoredActors() const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_DialogueState();


private:
	UPROPERTY(ReplicatedUsing = OnRep_DialogueState)
	bool bDialogueActive = false;
	
	UPROPERTY(ReplicatedUsing = OnRep_DialogueState)
	FDialogueStruct ActiveDialogue;

};
