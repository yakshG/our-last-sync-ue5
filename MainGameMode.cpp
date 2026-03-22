#include "MainGameMode.h"
#include "CoreGameInstance.h"
#include "SPlayerController.h"
#include "BaseRobot.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	UCoreGameInstance* GameInstance = GetGameInstance<UCoreGameInstance>();
	if (!GameInstance)
		return;

	if (GameInstance->CurrentMode == EMultiplayerMode::Local)
	{
		// Local Mode: both players on one machine
		// Create the second real local player immediately
		FString OutError;
		ULocalPlayer* NewLocalPlayer = GetGameInstance()->CreateLocalPlayer(1, OutError, true);
		if (NewLocalPlayer)
		{
			NewLocalPlayer->SetControllerId(1);
			UE_LOG(LogTemp, Warning, TEXT("[MainGameMode] BeginPlay: second local player created."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[MainGameMode] BeginPlay: failed to create second local player: %s"), *OutError);
		}
	}
}

void AMainGameMode::PostLogin(APlayerController* NewPlayer)
{

	ASPlayerController* SPC = Cast<ASPlayerController>(NewPlayer);
	if (!SPC)
		return;

	int32 AssignedIndex = RealPlayers.Num();
	RealPlayers.Add(SPC);

	if (RobotClasses.IsValidIndex(AssignedIndex))
	{
		SPC->SelectedRobotIndex = AssignedIndex;
		UE_LOG(LogTemp, Warning, TEXT("PostLogin: Player %d assigned to index %d"), RealPlayers.Num(), AssignedIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PostLogin: No RobotClass at index %d."), AssignedIndex);
	}
	
	// called after the correct index is assigned to the PlayerController
	Super::PostLogin(NewPlayer);


	RestartPlayer(SPC);
}

UClass* AMainGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	ASPlayerController* PC = Cast<ASPlayerController>(InController);

	if (PC && RobotClasses.IsValidIndex(PC->SelectedRobotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainGameMode] GetDefaultPawnClass: spawning robot class: %d"), PC->SelectedRobotIndex);
		return RobotClasses[PC->SelectedRobotIndex].Get();
	}

	if (RobotClasses.IsValidIndex(0))
	{
		UE_LOG(LogTemp, Warning, TEXT("fallback to index: 0"));
		return RobotClasses[0].Get();
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* AMainGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	ASPlayerController* SPC = Cast<ASPlayerController>(Player);
	if (!SPC) 
		return Super::ChoosePlayerStart_Implementation(Player);

	FName DesiredTag = (SPC->SelectedRobotIndex == 0) ? FName("Player1") : FName("Player2");

	TArray<AActor*> FoundStarts;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), APlayerStart::StaticClass(), DesiredTag, FoundStarts);

	if (FoundStarts.Num() > 0)
	{
		return FoundStarts[0];
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}