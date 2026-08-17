#include "StoneProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "KabulGameMode.h"
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
	ProjectileMovement->bSweepCollision = true;

	// Allow the stone to bounce after an impact.
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.32f;
	ProjectileMovement->Friction = 0.28f;
	ProjectileMovement->bBounceAngleAffectsFriction = true;
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = 250.0f;
	ProjectileMovement->BounceAdditionalIterations = 2;
	ProjectileMovement->bForceSubStepping = true;
	ProjectileMovement->MaxSimulationTimeStep = 0.025f;
	ProjectileMovement->MaxSimulationIterations = 8;

	ProjectileMovement->OnProjectileBounce.AddDynamic(
		this,
		&AStoneProjectile::HandleProjectileBounce
	);

	ProjectileMovement->OnProjectileStop.AddDynamic(
		this,
		&AStoneProjectile::HandleProjectileStop
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
	CollisionCount = 0;

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
	SetLifeSpan(MaximumLifetime);
}

void AStoneProjectile::HandleProjectileBounce(
	const FHitResult& ImpactResult,
	const FVector& ImpactVelocity
)
{
	++CollisionCount;

	AActor* HitActor = ImpactResult.GetActor();
	if (HitActor
		&& HitActor->GetClass()->GetName().Contains(TEXT("WobbleTarget")))
	{
		if (AKabulGameMode* GameMode =
			GetWorld() ? GetWorld()->GetAuthGameMode<AKabulGameMode>() : nullptr)
		{
			GameMode->NotifyWobbleTargetHit(HitActor);
		}
	}

	if (!bHasDamagedZombie
		&& ImpactVelocity.SizeSquared()
			>= FMath::Square(MinimumDamageSpeed))
	{
		if (AZombieCharacter* Zombie = Cast<AZombieCharacter>(HitActor))
		{
			bHasDamagedZombie = true;
			Zombie->ReceiveStoneImpact(ImpactResult, ImpactVelocity);

			// A body hit is the final collision. This keeps the real stone and
			// the body marker in the preview in exact agreement.
			if (ProjectileMovement)
			{
				ProjectileMovement->StopSimulating(ImpactResult);
			}
			return;
		}
	}

	if (ProjectileMovement
		&& CollisionCount >= MaximumCollisionCount)
	{
		ProjectileMovement->StopSimulating(ImpactResult);
	}
}

void AStoneProjectile::HandleProjectileStop(
	const FHitResult&
)
{
	if (Collision)
	{
		Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SetLifeSpan(StoppedLifetime);
}

float AStoneProjectile::GetCollisionRadius() const
{
	return Collision ? Collision->GetScaledSphereRadius() : 5.0f;
}

float AStoneProjectile::GetProjectileGravityScale() const
{
	return ProjectileMovement
		? ProjectileMovement->ProjectileGravityScale
		: 1.0f;
}

float AStoneProjectile::GetBounciness() const
{
	return ProjectileMovement ? ProjectileMovement->Bounciness : 0.32f;
}

float AStoneProjectile::GetFriction() const
{
	return ProjectileMovement ? ProjectileMovement->Friction : 0.28f;
}
