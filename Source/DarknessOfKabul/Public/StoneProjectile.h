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

private:
	/** The physical collision shape and root component. */
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Projectile",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<USphereComponent> Collision;

	/** The visible stone. Collision is handled by Collision above. */
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
};
