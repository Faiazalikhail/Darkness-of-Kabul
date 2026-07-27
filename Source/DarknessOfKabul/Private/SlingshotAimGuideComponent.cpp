#include "SlingshotAimGuideComponent.h"

#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

USlingshotAimGuideComponent::USlingshotAimGuideComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USlingshotAimGuideComponent::DrawGuide(
	const UCameraComponent* Camera,
	const bool bIsAiming,
	const bool bIsCharging,
	const float HeldTime,
	const float MinimumChargeTime,
	const float MaximumChargeTime,
	const float MinimumLaunchSpeed,
	const float MaximumLaunchSpeed
) const
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();

	if (!World || !Camera || !Owner || !bIsAiming)
	{
		return;
	}

	const FVector CameraLocation = Camera->GetComponentLocation();
	const FVector CameraForward = Camera->GetForwardVector();
	const FVector CameraUp = Camera->GetUpVector();

	FCollisionQueryParams QueryParameters(
		SCENE_QUERY_STAT(SlingshotAimGuide),
		false,
		Owner
	);

	QueryParameters.AddIgnoredActor(Owner);

	// Part 1: the small red dot shows the straight camera target.
	const FVector StraightTraceEnd =
		CameraLocation + CameraForward * TargetDistance;

	FHitResult StraightHit;

	const bool bStraightHit =
		World->LineTraceSingleByChannel(
			StraightHit,
			CameraLocation,
			StraightTraceEnd,
			ECC_Visibility,
			QueryParameters
		);

	const FVector TargetPoint =
		bStraightHit ? StraightHit.ImpactPoint : StraightTraceEnd;

	DrawDebugPoint(
		World,
		TargetPoint,
		6.0f,
		FColor::Red,
		false,
		0.0f,
		0
	);

	// Part 2: the curved guide begins below the target dot.
	const FVector TrajectoryStart =
		CameraLocation
		+ CameraForward * TrajectoryStartDistance
		- CameraUp * TrajectoryVerticalOffset;

	const FVector AimDirection =
		(TargetPoint - TrajectoryStart).GetSafeNormal();

	const float SafeMinimumTime =
		FMath::Max(MinimumChargeTime, KINDA_SMALL_NUMBER);

	const float ReadyAmount =
		bIsCharging
			? FMath::Clamp(HeldTime / SafeMinimumTime, 0.0f, 1.0f)
			: 0.0f;

	const float FullChargeRange =
		FMath::Max(
			MaximumChargeTime - MinimumChargeTime,
			KINDA_SMALL_NUMBER
		);

	const float FullChargeAmount =
		bIsCharging
			? FMath::Clamp(
				(HeldTime - MinimumChargeTime) / FullChargeRange,
				0.0f,
				1.0f
			)
			: 0.0f;

	// Before the shot is ready, preview a weak stone that drops quickly.
	// After it becomes ready, the increased speed naturally flattens the path.
	const float WeakPreviewSpeed = MinimumLaunchSpeed * 0.25f;

	const float PreviewSpeed =
		ReadyAmount < 1.0f
			? FMath::Lerp(
				WeakPreviewSpeed,
				MinimumLaunchSpeed,
				ReadyAmount
			)
			: FMath::Lerp(
				MinimumLaunchSpeed,
				MaximumLaunchSpeed,
				FullChargeAmount
			);

	const bool bShotIsReady =
		bIsCharging && HeldTime >= MinimumChargeTime;

	const FColor GuideColor =
		bShotIsReady
			? FColor(70, 255, 90)
			: FColor(255, 125, 25);

	const FVector LaunchVelocity = AimDirection * PreviewSpeed;
	const FVector Gravity(0.0f, 0.0f, World->GetGravityZ());

	FVector PreviousPoint = TrajectoryStart;

	for (int32 PointIndex = 1;
		PointIndex <= TrajectoryPointCount;
		++PointIndex)
	{
		const float Time = PointIndex * TrajectoryTimeStep;

		const FVector NextPoint =
			TrajectoryStart
			+ LaunchVelocity * Time
			+ 0.5f * Gravity * FMath::Square(Time);

		FHitResult PathHit;

		const bool bPathHit =
			World->LineTraceSingleByChannel(
				PathHit,
				PreviousPoint,
				NextPoint,
				ECC_Visibility,
				QueryParameters
			);

		const FVector DisplayPoint =
			bPathHit ? PathHit.ImpactPoint : NextPoint;

		DrawDebugLine(
			World,
			PreviousPoint,
			DisplayPoint,
			GuideColor,
			false,
			0.0f,
			0,
			bShotIsReady ? 3.0f : 2.0f
		);

		DrawDebugPoint(
			World,
			DisplayPoint,
			bShotIsReady ? 5.0f : 3.0f,
			GuideColor,
			false,
			0.0f,
			0
		);

		if (bPathHit)
		{
			break;
		}

		PreviousPoint = NextPoint;
	}
}
