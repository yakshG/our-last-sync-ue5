#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "SpaceAnomaly.generated.h"

UENUM(BlueprintType)
enum class EAnomalyType : uint8
{
	BlackHole UMETA(DisplayName = "BlackHole"),
	WhiteHole UMETA(DisplayName = "WhiteHole")
};

UCLASS()
class SINGULARITY_API ASpaceAnomaly : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpaceAnomaly();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anomaly Type")
	EAnomalyType AnomalyType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anomaly Phase")
	float ActiveDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anomaly Physics", meta = (ToolTip = "Gravitational parameter GM in game units."))
	float Mass = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anomaly Physics", meta = (ToolTip = "Gravitational parameter GM in game units."))
	float PullMultiplier = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anomaly Physics", meta = (ToolTip = "Scalar multiplier on white hole ejection impulse."))
	float EjectionForce = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anomaly Physics")
	float SpawnInterval = 0.5f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* AnomalyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* InfluenceSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SchwarzschildSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* AudioComponent;

private:
	void DestroyAnomaly();

	void RunBlackHoleBehaviour(float DeltaTime);

	void RunWhiteHoleBehaviour(float DeltaTime);
	void SpawnNextStoredActor();

	UPrimitiveComponent* GetPhysicsRoot(AActor* Actor) const;
};
