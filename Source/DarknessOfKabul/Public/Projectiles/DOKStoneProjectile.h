#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DOKStoneProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/** A stone moved by our analytical kinematics instead of ProjectileMovementComponent. */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API ADOKStoneProjectile : public AActor
{
	GENERATED_BODY()

public:
	ADOKStoneProjectile();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "DOK|Projectile")
	void InitializeProjectile(
		const FVector& Start,
		const FVector& InitialVelocity,
		const FVector& InGravity);

	UFUNCTION(BlueprintPure, Category = "DOK|Projectile")
	FVector CalculatePositionAtTime(float Time) const;

	UFUNCTION(BlueprintPure, Category = "DOK|Projectile")
	FVector CalculateVelocityAtTime(float Time) const;

	UFUNCTION(BlueprintCallable, Category = "DOK|Projectile")
	void AdvanceProjectile(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "DOK|Projectile")
	void ResolveImpact(const FHitResult& Hit);

	UFUNCTION(BlueprintPure, Category = "DOK|Projectile")
	float GetFlightTime() const { return FlightTime; }

	UFUNCTION(BlueprintPure, Category = "DOK|Projectile")
	FVector GetCurrentVelocity() const { return CalculateVelocityAtTime(FlightTime); }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DOK|Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DOK|Components")
	TObjectPtr<UStaticMeshComponent> StoneMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Projectile", meta = (ClampMin = "0.5"))
	float StoneRadius = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Projectile", meta = (ClampMin = "0.1"))
	float MaximumLifetime = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Damage", meta = (ClampMin = "0.0"))
	float BodyDamage = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Damage", meta = (ClampMin = "0.0"))
	float HeadDamage = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Damage")
	FName HeadBoneName = TEXT("head");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Debug")
	bool bDrawCollisionSweep = false;

private:
	FVector LaunchPosition = FVector::ZeroVector;
	FVector LaunchVelocity = FVector::ZeroVector;
	FVector Gravity = FVector(0.0, 0.0, -980.0);
	float FlightTime = 0.0f;
	bool bInitialized = false;
	bool bImpactResolved = false;
};
