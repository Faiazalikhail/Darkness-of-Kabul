#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Physics/DOKKinematicsLibrary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDOKKinematicsPositionTest,
	"DOK.Physics.Kinematics.GDDNumericalExample",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDOKKinematicsPositionTest::RunTest(const FString& Parameters)
{
	const FVector Start = FVector::ZeroVector;
	const FVector InitialVelocity(4000.0, 0.0, 0.0);
	const FVector Gravity(0.0, 0.0, -980.0);
	const float Time = 0.5f;

	const FVector Position = UDOKKinematicsLibrary::PositionAtTime(
		Start,
		InitialVelocity,
		Gravity,
		Time);
	const FVector Velocity = UDOKKinematicsLibrary::VelocityAtTime(
		InitialVelocity,
		Gravity,
		Time);

	TestTrue(
		TEXT("Position is 2,000 cm forward and 122.5 cm below the no-gravity line"),
		Position.Equals(FVector(2000.0, 0.0, -122.5), 0.01));
	TestTrue(
		TEXT("Velocity has gained 490 cm/s downward speed"),
		Velocity.Equals(FVector(4000.0, 0.0, -490.0), 0.01));

	const TArray<FVector> PreviewPoints = UDOKKinematicsLibrary::BuildTrajectoryPoints(
		Start,
		InitialVelocity,
		Gravity,
		0.5f,
		0.1f);
	TestEqual(TEXT("Preview includes start plus five future samples"), PreviewPoints.Num(), 6);
	TestTrue(
		TEXT("Final preview sample uses the same position equation"),
		PreviewPoints.Last().Equals(Position, 0.01));

	return true;
}

#endif
