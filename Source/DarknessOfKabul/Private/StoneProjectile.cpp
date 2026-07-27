#include "StoneProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AStoneProjectile::AStoneProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Collision is the root and detects projectile impacts.
	Collision = CreateDefaultSubobject<USphereComponent>(
		TEXT("ProjectileCollision")
	);

	SetRootComponent(Collision);

	Collision->InitSphereRadius(5.0f);
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Collision->SetSimulatePhysics(false);

	// StoneMesh is visual only.
	StoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(
		TEXT("StoneMesh")
	);

	StoneMesh->SetupAttachment(Collision);
	StoneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ProjectileMovement moves the collision root and attached mesh.
	ProjectileMovement =
		CreateDefaultSubobject<UProjectileMovementComponent>(
			TEXT("ProjectileMovement")
		);

	ProjectileMovement->SetUpdatedComponent(Collision);
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 12000.0f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;

	// Allow the stone to bounce after an impact.
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.25f;
	ProjectileMovement->Friction = 0.6f;
	ProjectileMovement->bForceSubStepping = true;
}

void AStoneProjectile::Launch(
	const FVector& Direction,
	const float Speed
)
{
	if (!ProjectileMovement || Direction.IsNearlyZero())
	{
		return;
	}

	const float SafeSpeed = FMath::Max(0.0f, Speed);

	ProjectileMovement->InitialSpeed = SafeSpeed;
	ProjectileMovement->MaxSpeed =
		FMath::Max(ProjectileMovement->MaxSpeed, SafeSpeed);

	ProjectileMovement->Velocity =
		Direction.GetSafeNormal() * SafeSpeed;

	ProjectileMovement->Activate(true);
}
