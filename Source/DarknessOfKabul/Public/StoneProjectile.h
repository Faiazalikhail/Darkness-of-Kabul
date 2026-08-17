#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoneProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

/**
 * A stone launched by the player's slingshot.
 *
 * Collision detects impacts.
 * StoneMesh displays the stone.
 * ProjectileMovement makes it fly and bounce.
 */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API AStoneProjectile : public AActor
{
	GENERATED_BODY()

public:
	AStoneProjectile();

	/**
	 * Launches the stone in the supplied direction.
	 *
	 * @param Direction Direction in which the stone should travel.
	 * @param Speed Launch speed measured in centimetres per second.
	 */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void Launch(const FVector& Direction, float Speed);

	float GetCollisionRadius() const;
	float GetProjectileGravityScale() const;
	float GetBounciness() const;
	float GetFriction() const;
	float GetMaximumLifetime() const { return MaximumLifetime; }
	int32 GetMaximumCollisionCount() const { return MaximumCollisionCount; }

private:
	/** Routes the first damaging zombie impact to the struck bone. */
	UFUNCTION()
	void HandleProjectileBounce(
		const FHitResult& ImpactResult,
		const FVector& ImpactVelocity
	);

	UFUNCTION()
	void HandleProjectileStop(const FHitResult& ImpactResult);

	/** Physical collision shape and root component. */
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Projectile",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<USphereComponent> Collision;

	/** Visible stone mesh. Collision is handled by Collision. */
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Projectile",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UStaticMeshComponent> StoneMesh;

	/** Controls the stone's speed, gravity and bouncing. */
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Projectile",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** Prevents a slow, nearly stopped stone from damaging a zombie. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Projectile|Damage",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0")
	)
	float MinimumDamageSpeed = 800.0f;

	/** Bounds the readable ricochet sequence and prevents endless vibration. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Projectile|Bounce",
		meta = (AllowPrivateAccess = "true", ClampMin = "0")
	)
	int32 MaximumCollisionCount = 2;

	/** Hard lifetime for stones that never reach the movement stop threshold. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Projectile|Lifetime",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.1")
	)
	float MaximumLifetime = 8.0f;

	/** Brief delay keeps the final resting impact readable before cleanup. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Projectile|Lifetime",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0")
	)
	float StoppedLifetime = 0.75f;

	/** A single stone can damage only one zombie once. */
	bool bHasDamagedZombie = false;
	int32 CollisionCount = 0;
};
