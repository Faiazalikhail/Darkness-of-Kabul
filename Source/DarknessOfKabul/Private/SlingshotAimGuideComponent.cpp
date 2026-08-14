#include "SlingshotAimGuideComponent.h"

#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Engine/World.h"
#include "ZombieCharacter.h"

namespace
{
	/** Matches the bounce terms used by UProjectileMovementComponent. */
	FVector CalculateBounceVelocity(
		FVector Velocity,
		const FVector& SurfaceNormal,
		const float Bounciness,
		const float Friction)
	{
		const float VelocityDotNormal = FVector::DotProduct(
			Velocity,
			SurfaceNormal
		);

		if (VelocityDotNormal > 0.0f)
		{
			return Velocity;
		}

		const FVector ReflectedNormal = SurfaceNormal * -VelocityDotNormal;
		Velocity += ReflectedNormal;

		const float TangentialSpeed = Velocity.Size();
		const float ScaledFriction = TangentialSpeed > KINDA_SMALL_NUMBER
			? FMath::Clamp(
				-VelocityDotNormal / TangentialSpeed,
				0.0f,
				1.0f
			) * Friction
			: Friction;

		Velocity *= FMath::Clamp(1.0f - ScaledFriction, 0.0f, 1.0f);
		Velocity += ReflectedNormal * FMath::Max(Bounciness, 0.0f);
		return Velocity;
	}
}

USlingshotAimGuideComponent::USlingshotAimGuideComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool USlingshotAimGuideComponent::PredictTrajectory(
	const FVector& StartLocation,
	const FVector& LaunchVelocity,
	const float ProjectileRadius,
	const float ProjectileGravityScale,
	const float Bounciness,
	const float Friction,
	const int32 MaximumCollisionCount,
	const float MaximumSimulationTime,
	const AActor* ActorToIgnore,
	FSlingshotTrajectoryPrediction& OutPrediction
) const
{
	UWorld* World = GetWorld();
	OutPrediction = FSlingshotTrajectoryPrediction();

	if (!World
		|| LaunchVelocity.IsNearlyZero()
		|| MaximumCollisionCount <= 0
		|| MaximumSimulationTime <= 0.0f)
	{
		return false;
	}

	FCollisionQueryParams QueryParameters(
		SCENE_QUERY_STAT(SlingshotTrajectory),
		false,
		ActorToIgnore
	);

	if (ActorToIgnore)
	{
		QueryParameters.AddIgnoredActor(ActorToIgnore);
	}

	const float Radius = FMath::Max(0.0f, ProjectileRadius);
	const FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);
	const FVector Gravity(
		0.0f,
		0.0f,
		World->GetGravityZ() * ProjectileGravityScale
	);
	const float TimeStep = 1.0f / FMath::Max(10.0f, SimulationFrequency);
	const int32 MaximumSteps = FMath::CeilToInt(
		MaximumSimulationTime / TimeStep
	);

	FVector Position = StartLocation;
	FVector Velocity = LaunchVelocity;

	for (int32 Step = 0; Step < MaximumSteps; ++Step)
	{
		const FVector NextVelocity = Velocity + Gravity * TimeStep;
		const FVector NextPosition = Position
			+ (Velocity + NextVelocity) * (0.5f * TimeStep);

		FHitResult Hit;
		if (!World->SweepSingleByChannel(
			Hit,
			Position,
			NextPosition,
			FQuat::Identity,
			ECC_GameTraceChannel1,
			Shape,
			QueryParameters))
		{
			Position = NextPosition;
			Velocity = NextVelocity;
			continue;
		}

		const FVector ImpactPoint = Hit.ImpactPoint;
		const FVector IncomingDirection = NextVelocity.GetSafeNormal();
		const FVector SurfaceNormal = Hit.ImpactNormal.GetSafeNormal();
		const FVector BounceVelocity = CalculateBounceVelocity(
			NextVelocity,
			SurfaceNormal,
			Bounciness,
			Friction
		);

		FSlingshotTrajectoryImpact& Impact =
			OutPrediction.Impacts.AddDefaulted_GetRef();
		Impact.ImpactPoint = ImpactPoint;
		Impact.IncomingGuidePoint =
			ImpactPoint - IncomingDirection * AngleGuideLength;
		Impact.OutgoingGuidePoint = ImpactPoint
			+ BounceVelocity.GetSafeNormal() * AngleGuideLength;
		Impact.HitActor = Hit.GetActor();
		Impact.bHitZombie = Cast<AZombieCharacter>(Hit.GetActor()) != nullptr;
		Impact.bHasOutgoingSegment = !Impact.bHitZombie
			&& OutPrediction.Impacts.Num() < MaximumCollisionCount
			&& !BounceVelocity.IsNearlyZero();

		OutPrediction.FinalPoint = ImpactPoint;
		OutPrediction.bEndedOnCollision = true;

		if (Impact.bHitZombie
			|| OutPrediction.Impacts.Num() >= MaximumCollisionCount
			|| BounceVelocity.SizeSquared() < FMath::Square(250.0f))
		{
			return true;
		}

		Position = ImpactPoint
			+ SurfaceNormal * FMath::Max(Radius + 0.5f, 1.0f);
		Velocity = BounceVelocity;
	}

	OutPrediction.FinalPoint = Position;
	OutPrediction.bEndedOnCollision = false;
	return true;
}
