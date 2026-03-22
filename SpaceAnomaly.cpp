#include "SpaceAnomaly.h"
#include "SGameState.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ASpaceAnomaly::ASpaceAnomaly()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	AnomalyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Black Hole"));
	SetRootComponent(AnomalyMesh);
	AnomalyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InfluenceSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InfluenceSphere"));
	InfluenceSphere->SetupAttachment(RootComponent);
	InfluenceSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InfluenceSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
	InfluenceSphere->InitSphereRadius(Mass * 6.0f);

	SchwarzschildSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SchwarzchildSphere"));
	SchwarzschildSphere->SetupAttachment(RootComponent);
	SchwarzschildSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SchwarzschildSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
	SchwarzschildSphere->InitSphereRadius(Mass * 1.25f);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->bAutoActivate = false;

	ActiveDuration = 5.0f;
}

void ASpaceAnomaly::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
		return;


	FTimerHandle LifetimeHandle;
	GetWorldTimerManager().SetTimer(LifetimeHandle, this, &ASpaceAnomaly::DestroyAnomaly, ActiveDuration, false);

	if (AnomalyType == EAnomalyType::WhiteHole)
	{
		FTimerHandle SpawnActorsHandle;
		GetWorldTimerManager().SetTimer(SpawnActorsHandle, this, &ASpaceAnomaly::SpawnNextStoredActor, SpawnInterval, true);
	}
}

void ASpaceAnomaly::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

}

void ASpaceAnomaly::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
		return;

	(AnomalyType == EAnomalyType::BlackHole) ? RunBlackHoleBehaviour(DeltaTime) : RunWhiteHoleBehaviour(DeltaTime);

}

void ASpaceAnomaly::DestroyAnomaly()
{
	if (!HasAuthority())
		return;

	Destroy();
}

UPrimitiveComponent* ASpaceAnomaly::GetPhysicsRoot(AActor* Actor) const
{
	if (!Actor) return nullptr;

	// Try root component first — covers AStaticMeshActor and Blueprint actors
	// where the static mesh is the root
	UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Actor->GetRootComponent());

	// Fallback: root is a plain USceneComponent, physics body is a child component
	if (!Root)
		Root = Actor->FindComponentByClass<UPrimitiveComponent>();

	// Only return if physics simulation is enabled
	if (Root && Root->IsSimulatingPhysics())
		return Root;

	return nullptr;
}

void ASpaceAnomaly::RunBlackHoleBehaviour(float DeltaTime)
{
	TArray<AActor*> OverlappingActors;

	// Polled each tick as actors enter/exit dynamically during short lifetime
	InfluenceSphere->GetOverlappingActors(OverlappingActors);

	ASGameState* GameState = GetWorld()->GetGameState<ASGameState>();
	if (!GameState)
		return;

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor) continue;
		if (Actor->IsA<ASpaceAnomaly>()) continue;

		UPrimitiveComponent* Root = GetPhysicsRoot(Actor);
		if (!Root) continue;

		FVector ToSingularity = GetActorLocation() - Root->GetCenterOfMass();
		float Dist = ToSingularity.Size();

		if (SchwarzschildSphere->IsOverlappingActor(Actor))
		{
			Root->SetSimulatePhysics(false);
			Root->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Actor->SetActorHiddenInGame(true);
			Actor->SetActorEnableCollision(false);
			Actor->SetActorTickEnabled(false);

			GameState->AddStoredActor(Actor);
			continue;
		}

		float PhotonSphereRadius = 3.0f * Mass;
		float PhotonMultiplier = (Dist < PhotonSphereRadius) ? FMath::Lerp(1.0f, 5.0f, 1.0f - (Dist / PhotonSphereRadius)) : 1.0f;

		FVector Dir = ToSingularity / FMath::Max(Dist, 1.0f);

		// Force scaled by 10000 to compensate for cm units;
		// PullMultiplier is tuned in Blueprints at 10^7 range,
		// divided here for designer-friendly values
		float ForceMag = PullMultiplier * 10000.0f * PhotonMultiplier * (Mass / FMath::Square(Dist));


		Root->AddForceAtLocation(Dir * ForceMag, Root->GetCenterOfMass());
	}
}

void ASpaceAnomaly::RunWhiteHoleBehaviour(float DeltaTime)
{
	TArray<AActor*> OverlappingActors;
	InfluenceSphere->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor) continue;
		if (Actor->IsA<ASpaceAnomaly>()) continue;

		UPrimitiveComponent* Root = GetPhysicsRoot(Actor);
		if (!Root) continue;

		FVector AwayFromSingularity = Root->GetCenterOfMass() - GetActorLocation();
		float Dist = AwayFromSingularity.Size();

		if (SchwarzschildSphere->IsOverlappingActor(Actor))
		{
			Root->AddImpulse((AwayFromSingularity / Dist) * EjectionForce * Root->GetMass(), NAME_None, true);
			continue;
		}

		float ForceMag = PullMultiplier * 10000.0f * (Mass / FMath::Square(Dist));

		Root->AddForceAtLocation((AwayFromSingularity / Dist) * ForceMag, Root->GetCenterOfMass());
	}
}

void ASpaceAnomaly::SpawnNextStoredActor()
{
	if (!HasAuthority()) return;

	ASGameState* GameState = GetWorld()->GetGameState<ASGameState>();
	if (!GameState || !GameState->HasStoredActors()) return;

	AActor* Actor = nullptr;
	if (!GameState->SpawnConsumedActor(Actor)) return;
	if (!IsValid(Actor)) return;

	Actor->SetActorLocation(GetActorLocation());
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorTickEnabled(true);

	UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
	if (!Root)
	{
		Root = Actor->FindComponentByClass<UPrimitiveComponent>();
	}

	if (Root)
	{
		Root->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Root->SetSimulatePhysics(true);

		Root->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Root->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		FVector RandomDir = FMath::VRand();
		RandomDir.Z = FMath::Abs(RandomDir.Z);
		RandomDir.Normalize();
		Root->AddImpulse(RandomDir * EjectionForce, NAME_None, true);
	}
}

