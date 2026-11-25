// Projectile of a shot: flies along its spawn forward, damages receivers it hits and can pierce them,
// losing a share of its damage on every hit; expires after a travel distance.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Shooting/ShotDamage.h"
#include "ShotProjectile.generated.h"

class UShotProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

/** Everything a projectile needs from the attack that fired it. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FProjectileShotParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	FShotDamage Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ClampMin = 1, Units = "cm/s"))
	float Speed = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float GravityScale = 0.f;

	/** Path length after which the projectile is destroyed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ClampMin = 0, Units = "cm"))
	float MaxTravelDistance = 10000.f;

	/** Share of the remaining damage lost on every hit; 1 = stops at the first hit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ClampMin = 0, ClampMax = 1))
	float DamageLossPerHit = 1.f;
};

UCLASS()
class TYPICALSHOOTER_API AShotProjectile : public AActor
{
	GENERATED_BODY()

public:
	AShotProjectile();

	/** Spawns a projectile flying along SpawnTransform's forward axis. Call on the server. */
	static AShotProjectile* SpawnShot(UWorld* World, TSubclassOf<AShotProjectile> Class, const FTransform& SpawnTransform,
		const FProjectileShotParams& Params, AActor* Owner = nullptr, APawn* Instigator = nullptr);

	/** Called on every blocking hit: applies damage (server) and loses a share of it. True = keep flying through. */
	bool HandleShotHit(const FHitResult& Hit);

	const FShotDamage& GetRemainingDamage() const { return Params.Damage; }
	float GetTraveledDistance() const { return TraveledDistance; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void ApplyParams();
	void Expire();

	UFUNCTION()
	void OnStopped(const FHitResult& Hit);

	UFUNCTION()
	void OnRep_Flight();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UShotProjectileMovementComponent> Movement;

	/** Server-side shot; remaining damage lives in Params.Damage. */
	UPROPERTY()
	FProjectileShotParams Params;

	/** Flight parameters replicated once, so clients simulate the same trajectory (x = speed, y = gravity scale). */
	UPROPERTY(ReplicatedUsing = OnRep_Flight)
	FVector2D Flight = FVector2D::ZeroVector;

	FVector LastLocation = FVector::ZeroVector;
	float TraveledDistance = 0.f;
};
