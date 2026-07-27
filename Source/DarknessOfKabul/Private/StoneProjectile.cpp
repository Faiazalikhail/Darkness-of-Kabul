#include "StoneProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AStoneProjectile::AStoneProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// The collision sphere is the root and is responsible for detecting hits.
	Collision = CreateDefaultSubobject<USphereComponent>(
		TEXT("ProjectileCollision")
	);
	SetRootComponent(Collision);

	Collision->InitSphereRadius(5.0f);
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Collision->SetSimulatePhysics(false);

	// The stone mesh is visual only, so it does not need its own collision.
	StoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(
		TEXT("StoneMesh")
	);
	StoneMesh->SetupAttachment(Collision);
	StoneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// This component moves the collision sphere and everything attached to it.
	ProjectileMovement =
		CreateDefaultSubobject<UProjectileMovementComponent>(
			TEXT("ProjectileMovement")
		);

	ProjectileMovement->SetUpdatedComponent(Collision);
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 6000.0f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;

	// Allow the stone to bounce when it strikes something.
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.25f;
	ProjectileMovement->Friction = 0.6f;
	ProjectileMovement->bForceSubStepping = true;
}
