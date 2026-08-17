#include "KinematicsLibrary.h"

float UKinematicsLibrary::CalculateChargeAmount(
	const float HeldTime,
	const float MinimumChargeTime,
	const float MaximumChargeTime
)
{
	const float ChargeRange = FMath::Max(
		MaximumChargeTime - MinimumChargeTime,
		KINDA_SMALL_NUMBER
	);

	return FMath::Clamp(
		(HeldTime - MinimumChargeTime) / ChargeRange,
		0.0f,
		1.0f
	);
}

float UKinematicsLibrary::CalculateLaunchSpeed(
	const float HeldTime,
	const float MinimumChargeTime,
	const float MaximumChargeTime,
	const float MinimumLaunchSpeed,
	const float MaximumLaunchSpeed
)
{
	return FMath::Lerp(
		MinimumLaunchSpeed,
		MaximumLaunchSpeed,
		CalculateChargeAmount(
			HeldTime,
			MinimumChargeTime,
			MaximumChargeTime
		)
	);
}
