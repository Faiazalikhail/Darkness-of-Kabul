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

private:
	/** Routes the first damaging zombie impact to the struck bone. */
	UFUNCTION()
	void HandleProjectileBounce(
		const FHitResult& ImpactResult,
		const FVector& ImpactVelocity
	);

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

	/** A single stone can damage only one zombie once. */
	bool bHasDamagedZombie = false;
};
