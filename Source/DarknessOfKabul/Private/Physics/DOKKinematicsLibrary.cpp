#include "Physics/DOKKinematicsLibrary.h"

FVector UDOKKinematicsLibrary::PositionAtTime(
	const FVector& StartPosition,
	const FVector& InitialVelocity,
	const FVector& Acceleration,
	const float Time)
{
	const float SafeTime = FMath::Max(0.0f, Time);
	return StartPosition
		+ InitialVelocity * SafeTime
		+ 0.5f * Acceleration * SafeTime * SafeTime;
}

FVector UDOKKinematicsLibrary::VelocityAtTime(
	const FVector& InitialVelocity,
	const FVector& Acceleration,
	const float Time)
{
	return InitialVelocity + Acceleration * FMath::Max(0.0f, Time);
}

TArray<FVector> UDOKKinematicsLibrary::BuildTrajectoryPoints(
	const FVector& StartPosition,
	const FVector& InitialVelocity,
	const FVector& Acceleration,
	const float MaxTime,
	const float TimeStep)
{
	TArray<FVector> Points;
	const float SafeMaxTime = FMath::Max(0.0f, MaxTime);
	const float SafeTimeStep = FMath::Max(0.001f, TimeStep);
	const int32 PointCount = FMath::FloorToInt(SafeMaxTime / SafeTimeStep) + 1;

	Points.Reserve(PointCount);
	for (int32 Index = 0; Index < PointCount; ++Index)
	{
		const float Time = FMath::Min(Index * SafeTimeStep, SafeMaxTime);
		Points.Add(PositionAtTime(StartPosition, InitialVelocity, Acceleration, Time));
	}

	return Points;
}
