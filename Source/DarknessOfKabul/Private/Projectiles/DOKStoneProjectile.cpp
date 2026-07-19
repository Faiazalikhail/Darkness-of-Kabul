#include "Projectiles/DOKStoneProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Physics/DOKKinematicsLibrary.h"
#include "UObject/ConstructorHelpers.h"

ADOKStoneProjectile::ADOKStoneProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(StoneRadius);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionSphere->SetCollisionProfileName(TEXT("StoneProjectile"));

	StoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneMesh"));
	StoneMesh->SetupAttachment(CollisionSphere);
	StoneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StoneMesh->SetRelativeScale3D(FVector(0.06f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		StoneMesh->SetStaticMesh(SphereMesh.Object);
	}
}

void ADOKStoneProjectile::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	AdvanceProjectile(DeltaTime);
}

void ADOKStoneProjectile::InitializeProjectile(
	const FVector& Start,
	const FVector& InitialVelocity,
	const FVector& InGravity)
{
	LaunchPosition = Start;
	LaunchVelocity = InitialVelocity;
	Gravity = InGravity;
	FlightTime = 0.0f;
	bInitialized = true;
	bImpactResolved = false;

	SetActorLocation(Start);
	SetActorRotation(InitialVelocity.Rotation());
	CollisionSphere->SetSphereRadius(StoneRadius);
	SetActorTickEnabled(true);
}

FVector ADOKStoneProjectile::CalculatePositionAtTime(const float Time) const
{
	return UDOKKinematicsLibrary::PositionAtTime(LaunchPosition, LaunchVelocity, Gravity, Time);
}

FVector ADOKStoneProjectile::CalculateVelocityAtTime(const float Time) const
{
	return UDOKKinematicsLibrary::VelocityAtTime(LaunchVelocity, Gravity, Time);
}

void ADOKStoneProjectile::AdvanceProjectile(const float DeltaTime)
{
	if (!bInitialized || bImpactResolved || !GetWorld())
	{
		return;
	}

	const FVector PreviousPosition = GetActorLocation();
	FlightTime += FMath::Max(0.0f, DeltaTime);

	if (FlightTime >= MaximumLifetime)
	{
		Destroy();
		return;
	}

	const FVector NewPosition = CalculatePositionAtTime(FlightTime);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DOKStoneSweep), true, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bReturnPhysicalMaterial = true;

	FHitResult Hit;
	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		PreviousPosition,
		NewPosition,
		FQuat::Identity,
		ECC_GameTraceChannel1,
		FCollisionShape::MakeSphere(StoneRadius),
		QueryParams);

	if (bDrawCollisionSweep)
	{
		DrawDebugLine(GetWorld(), PreviousPosition, NewPosition, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.0f);
		DrawDebugSphere(GetWorld(), NewPosition, StoneRadius, 8, bHit ? FColor::Red : FColor::Green, false, 1.0f);
	}

	if (bHit)
	{
		SetActorLocation(Hit.Location);
		ResolveImpact(Hit);
		return;
	}

	SetActorLocation(NewPosition);
	SetActorRotation(CalculateVelocityAtTime(FlightTime).Rotation());
}

void ADOKStoneProjectile::ResolveImpact(const FHitResult& Hit)
{
	if (bImpactResolved)
	{
		return;
	}

	bImpactResolved = true;
	SetActorTickEnabled(false);

	if (AActor* HitActor = Hit.GetActor())
	{
		const bool bHeadHit = Hit.BoneName == HeadBoneName;
		const float Damage = bHeadHit ? HeadDamage : BodyDamage;
		UGameplayStatics::ApplyPointDamage(
			HitActor,
			Damage,
			CalculateVelocityAtTime(FlightTime).GetSafeNormal(),
			Hit,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass());
	}

	SetActorHiddenInGame(true);
	SetLifeSpan(0.01f);
}
