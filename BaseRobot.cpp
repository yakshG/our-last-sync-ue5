#include "BaseRobot.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Interact.h"
#include "SPlayerController.h"

ABaseRobot::ABaseRobot()
{
 	PrimaryActorTick.bCanEverTick = true;
	
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	LookOffset = 0.f;
	LineTraceLength = 0.f;
	HasPickedUp = false;
	ActorToPickUp = nullptr;
	WidgetClass = nullptr;
	CanMove = true;
	HeldPrimitive = nullptr;
}

void ABaseRobot::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	CameraBoom->SetRelativeRotation(FRotator(-20.f, 0.f, 0.f));
}

void ABaseRobot::BeginPlay()
{
	Super::BeginPlay();

}

void ABaseRobot::Move(const FInputActionValue& Value)
{
	if (!CanMove)
		return;

	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ABaseRobot::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X * LookOffset);
	AddControllerPitchInput(LookAxisVector.Y * LookOffset);

}

void ABaseRobot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

void ABaseRobot::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ABaseRobot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast <UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaseRobot::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaseRobot::Look);

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABaseRobot::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABaseRobot::StopJumping);
	}

}

void ABaseRobot::SpawnAnomaly()
{
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + FollowCamera->GetForwardVector() * LineTraceLength;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FVector SpawnLocation = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params) ? Hit.ImpactPoint + Hit.ImpactNormal * 60.f : End;

	if (!HasAuthority())
	{
		Server_SpawnAnomaly(SpawnLocation);
		return;
	}

	if (AnomalyClass)
	{
		GetWorld()->SpawnActor<ASpaceAnomaly>(AnomalyClass, SpawnLocation, FRotator::ZeroRotator);
	}
}

void ABaseRobot::Server_SpawnAnomaly_Implementation(const FVector& SpawnLocation)
{
	if (AnomalyClass)
	{
		GetWorld()->SpawnActor<ASpaceAnomaly>(AnomalyClass, SpawnLocation, FRotator::ZeroRotator);
	}
}

void ABaseRobot::PickupDropObject()
{
	if (!FollowCamera) return;

	if (!HasPickedUp)
	{
		FVector Start = FollowCamera->GetComponentLocation() + FVector(0.f, 0.f, -100.f);
		FVector End = Start + (FollowCamera->GetForwardVector() * LineTraceLength);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params);
		if (bHit && Hit.GetActor() && Hit.GetActor()->GetClass()->ImplementsInterface(UInteract::StaticClass()))
		{
			ActorToPickUp = Hit.GetActor();
			HasPickedUp = true;
			if (IsValid(ActorToPickUp))
			{
				HeldPrimitive = Cast<UPrimitiveComponent>(ActorToPickUp->GetRootComponent());
				if (HeldPrimitive)
				{
					HeldPrimitive->SetSimulatePhysics(false);
					HeldPrimitive->SetEnableGravity(false);

					ActorToPickUp->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
					UE_LOG(LogTemp, Warning, TEXT("picked up an interactable actor"));
				}
			}
		}
	}

	else
	{
		if (IsValid(ActorToPickUp))
		{
			if (HeldPrimitive)
			{
				HeldPrimitive->SetSimulatePhysics(true);
				HeldPrimitive->SetEnableGravity(true);
			}
			ActorToPickUp->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			ActorToPickUp = nullptr;
			HasPickedUp = false;
			UE_LOG(LogTemp, Warning, TEXT("actor detached!"));
		}
	}
}