#include "SPlayerController.h"
#include "BaseRobot.h"
#include "SGameState.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"

void ASPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("[SPlayerController] BeginPlay: selected robot index: %d"), SelectedRobotIndex);
	
	UE_LOG(LogTemp, Warning, TEXT("[SPlayerController] : %s"), *GetName());

	FTimerHandle DelayTimer;
	GetWorldTimerManager().SetTimer(DelayTimer, this, &ASPlayerController::CreatePlayerWidget, 2.0f, false);

}


void ASPlayerController::CreatePlayerWidget()
{

	// only local machine's own controller can create screen widgets
	// on the server, client's PC exists but is not local

	if (!IsLocalController()) return;

	PlayerPawn = Cast<ABaseRobot>(GetPawn());
	if (!PlayerPawn)
		return;

	if (PlayerPawn->PlayerID)
	{
		ControlledPawnID = PlayerPawn->PlayerID;
		UE_LOG(LogTemp, Warning, TEXT("player ID set: %d"), ControlledPawnID);
	}

	if (PlayerPawn->WidgetClass)
	{
		PlayerWidget = CreateWidget<UPlayerWidget>(this, PlayerPawn->WidgetClass);
		if (PlayerWidget)
		{
			PlayerWidget->PlayerID = ControlledPawnID;
			PlayerWidget->AddToPlayerScreen();
		}
	}

	ASGameState* GameState = GetWorld()->GetGameState<ASGameState>();
	if (GameState)
	{
		GameState->OnDialogueStateChanged.AddUObject(this, &ASPlayerController::HandleDialogueStateChanged);
	}
	
}

void ASPlayerController::HandleDialogueStateChanged()
{
	if (!PlayerWidget) return;

	ASGameState* GameState = GetWorld()->GetGameState<ASGameState>();

	if (!GameState) return;

	if (GameState->IsDialogueActive())
	{
		PlayerWidget->ShowDialogue(GameState->GetActiveDialogue());
	}
	else
	{
		PlayerWidget->HideDialogue();
	}
}

void ASPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPlayerController, SelectedRobotIndex);
}