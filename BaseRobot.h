#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainGameMode.h"
#include "SpaceAnomaly.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "BaseRobot.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class SINGULARITY_API ABaseRobot : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

public:
	ABaseRobot();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	float LookOffset;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Ability")
	TSubclassOf<ASpaceAnomaly> AnomalyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	AActor* ActorToPickUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float LineTraceLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool HasPickedUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 PlayerID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool CanMove;

	UPrimitiveComponent* HeldPrimitive;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void NotifyControllerChanged() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SpawnAnomaly(const FVector& SpawnLocation);

private:
	AMainGameMode* GameMode;

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void SpawnAnomaly();

	UFUNCTION(BlueprintCallable)
	void PickupDropObject();
	
};
