#include "StoneProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ZombieCharacter.h"

AStoneProjectile::AStoneProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Collision is the root and detects projectile impacts.
	Collision = CreateDefaultSubobject<USphereComponent>(
		TEXT("ProjectileCollision")
	);

	SetRootComponent(Collision);

	Collision->InitSphereRadius(5.0f);
	Collision->SetCollisionProfileName(TEXT("StoneProjectile"));
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
	ProjectileMovement->InitialSpeed = 3500.0f;
	ProjectileMovement->MaxSpeed = 8500.0f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;

	// Allow the stone to bounce after an impact.
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.25f;
	ProjectileMovement->Friction = 0.4f;
	ProjectileMovement->bForceSubStepping = true;

	ProjectileMovement->OnProjectileBounce.AddDynamic(
		this,
		&AStoneProjectile::HandleProjectileBounce
	);
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
	bHasDamagedZombie = false;

	if (AActor* ProjectileOwner = GetOwner())
	{
		Collision->IgnoreActorWhenMoving(ProjectileOwner, true);
	}

	if (APawn* ProjectileInstigator = GetInstigator())
	{
		Collision->IgnoreActorWhenMoving(ProjectileInstigator, true);
	}

	ProjectileMovement->InitialSpeed = SafeSpeed;
	ProjectileMovement->MaxSpeed =
		FMath::Max(ProjectileMovement->MaxSpeed, SafeSpeed);

	ProjectileMovement->Velocity =
		Direction.GetSafeNormal() * SafeSpeed;

	ProjectileMovement->Activate(true);
}

void AStoneProjectile::HandleProjectileBounce(
	const FHitResult& ImpactResult,
	const FVector& ImpactVelocity
)
{
	if (bHasDamagedZombie
		|| ImpactVelocity.SizeSquared()
			< FMath::Square(MinimumDamageSpeed))
	{
		return;
	}

	AZombieCharacter* Zombie =
		Cast<AZombieCharacter>(ImpactResult.GetActor());

	if (!Zombie)
	{
		return;
	}

	bHasDamagedZombie = true;
	Zombie->ReceiveStoneImpact(ImpactResult, ImpactVelocity);
}
