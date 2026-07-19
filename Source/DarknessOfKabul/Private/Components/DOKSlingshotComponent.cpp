#include "Components/DOKSlingshotComponent.h"

#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Physics/DOKKinematicsLibrary.h"
#include "Projectiles/DOKStoneProjectile.h"

UDOKSlingshotComponent::UDOKSlingshotComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	StoneClass = ADOKStoneProjectile::StaticClass();
}

void UDOKSlingshotComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = FMath::Clamp(StartingAmmo, 0, MaximumAmmo);
	OnAmmoChanged.Broadcast(CurrentAmmo);
}

void UDOKSlingshotComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDrawing)
	{
		OnDrawChanged.Broadcast(GetDrawPercent());
		if (bTrajectoryGuideEnabled)
		{
			DrawTrajectoryPreview();
		}
	}
}

bool UDOKSlingshotComponent::StartDrawing()
{
	if (bIsDrawing || CurrentAmmo <= 0 || !GetWorld())
	{
		return false;
	}

	if (GetWorld()->GetTimeSeconds() - LastFireTime < FireCooldown)
	{
		return false;
	}

	bIsDrawing = true;
	DrawStartTime = GetWorld()->GetTimeSeconds();
	OnDrawChanged.Broadcast(0.0f);
	return true;
}

void UDOKSlingshotComponent::CancelDrawing()
{
	bIsDrawing = false;
	OnDrawChanged.Broadcast(0.0f);
}

bool UDOKSlingshotComponent::ReleaseShot(const FVector& AimDirection)
{
	if (!bIsDrawing || CurrentAmmo <= 0 || !GetWorld() || !StoneClass)
	{
		CancelDrawing();
		return false;
	}

	const float LaunchSpeed = CalculateLaunchSpeed();
	const FVector SafeAimDirection = AimDirection.GetSafeNormal();
	const FVector SpawnLocation = GetLaunchPosition(SafeAimDirection);
	const FVector InitialVelocity = SafeAimDirection * LaunchSpeed;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = Cast<APawn>(GetOwner());
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADOKStoneProjectile* Stone = GetWorld()->SpawnActor<ADOKStoneProjectile>(
		StoneClass,
		SpawnLocation,
		SafeAimDirection.Rotation(),
		SpawnParameters);

	if (!Stone)
	{
		CancelDrawing();
		return false;
	}

	Stone->InitializeProjectile(SpawnLocation, InitialVelocity, Gravity);
	--CurrentAmmo;
	LastFireTime = GetWorld()->GetTimeSeconds();
	bIsDrawing = false;

	OnAmmoChanged.Broadcast(CurrentAmmo);
	OnDrawChanged.Broadcast(0.0f);
	OnShotReleased.Broadcast(LaunchSpeed);
	return true;
}

float UDOKSlingshotComponent::GetDrawPercent() const
{
	if (!bIsDrawing || !GetWorld())
	{
		return 0.0f;
	}

	const float DrawTime = GetWorld()->GetTimeSeconds() - DrawStartTime;
	return FMath::Clamp(DrawTime / FMath::Max(0.01f, MaximumDrawTime), 0.0f, 1.0f);
}

float UDOKSlingshotComponent::CalculateLaunchSpeed() const
{
	return FMath::Lerp(MinimumLaunchSpeed, MaximumLaunchSpeed, GetDrawPercent());
}

int32 UDOKSlingshotComponent::AddAmmo(const int32 Amount)
{
	if (Amount <= 0)
	{
		return 0;
	}

	const int32 OldAmmo = CurrentAmmo;
	CurrentAmmo = FMath::Clamp(CurrentAmmo + Amount, 0, MaximumAmmo);
	const int32 AddedAmmo = CurrentAmmo - OldAmmo;

	if (AddedAmmo > 0)
	{
		OnAmmoChanged.Broadcast(CurrentAmmo);
	}

	return AddedAmmo;
}

void UDOKSlingshotComponent::ToggleTrajectoryGuide()
{
	bTrajectoryGuideEnabled = !bTrajectoryGuideEnabled;
}

TArray<FVector> UDOKSlingshotComponent::BuildTrajectoryPreview(const FVector& AimDirection) const
{
	const FVector SafeAimDirection = AimDirection.GetSafeNormal();
	return UDOKKinematicsLibrary::BuildTrajectoryPoints(
		GetLaunchPosition(SafeAimDirection),
		SafeAimDirection * CalculateLaunchSpeed(),
		Gravity,
		PreviewTime,
		PreviewTimeStep);
}

FVector UDOKSlingshotComponent::GetLaunchPosition(const FVector& AimDirection) const
{
	if (const AActor* Owner = GetOwner())
	{
		if (const UCameraComponent* Camera = Owner->FindComponentByClass<UCameraComponent>())
		{
			return Camera->GetComponentLocation() + AimDirection * SpawnDistanceFromCamera;
		}
		return Owner->GetActorLocation() + AimDirection * SpawnDistanceFromCamera;
	}
	return FVector::ZeroVector;
}

void UDOKSlingshotComponent::DrawTrajectoryPreview() const
{
	const AActor* Owner = GetOwner();
	const UCameraComponent* Camera = Owner ? Owner->FindComponentByClass<UCameraComponent>() : nullptr;
	if (!Camera || !GetWorld())
	{
		return;
	}

	const TArray<FVector> Points = BuildTrajectoryPreview(Camera->GetForwardVector());
	for (int32 Index = 0; Index < Points.Num(); ++Index)
	{
		DrawDebugSphere(GetWorld(), Points[Index], 3.0f, 6, FColor::Cyan, false, 0.0f, 0, 0.75f);
		if (Index > 0)
		{
			DrawDebugLine(GetWorld(), Points[Index - 1], Points[Index], FColor::Cyan, false, 0.0f, 0, 0.5f);
		}
	}
}
