#include "ZombieCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KabulGameMode.h"
#include "TimerManager.h"

AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Stones must pass through the broad navigation capsule and strike the
	// animated bodies in the mesh's Physics Asset instead.
	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_GameTraceChannel1,
		ECR_Ignore
	);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(
		ECC_GameTraceChannel1,
		ECR_Block
	);
}

void AZombieCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	PhysicalState = EZombiePhysicalState::Standing;
	InitialActorTransform = GetActorTransform();
	InitialMeshRelativeTransform = GetMesh()->GetRelativeTransform();
	InitialCapsuleCollisionProfile =
		GetCapsuleComponent()->GetCollisionProfileName();
	InitialMeshCollisionProfile = GetMesh()->GetCollisionProfileName();
	InitialCapsuleCollisionEnabled =
		GetCapsuleComponent()->GetCollisionEnabled();
	InitialMeshCollisionEnabled = GetMesh()->GetCollisionEnabled();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		StandingMaxWalkSpeed = Movement->MaxWalkSpeed;
	}

	// Enforce these responses at runtime because an existing Blueprint child
	// may have serialized older component defaults.
	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_GameTraceChannel1,
		ECR_Ignore
	);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(
		ECC_GameTraceChannel1,
		ECR_Block
	);
}

void AZombieCharacter::ReceiveStoneImpact(
	const FHitResult& Hit,
	const FVector& ImpactVelocity
)
{
	if (PhysicalState == EZombiePhysicalState::Dead)
	{
		return;
	}

	FHitResult ResolvedHit = Hit;

	if (ResolvedHit.BoneName.IsNone() && GetMesh())
	{
		ResolvedHit.BoneName = GetMesh()->FindClosestBone(
			ResolvedHit.bBlockingHit
				? ResolvedHit.ImpactPoint
				: GetActorLocation(),
			nullptr,
			0.0f,
			true
		);
	}

	const EZombieHitZone HitZone =
		ClassifyHitBone(ResolvedHit.BoneName);
	LastHitZone = HitZone;

	float Damage = 0.0f;

	switch (HitZone)
	{
	case EZombieHitZone::Head:
		Damage = CurrentHealth;
		break;

	case EZombieHitZone::Torso:
		Damage = TorsoDamage;
		break;

	case EZombieHitZone::Arm:
		Damage = ArmDamage;
		break;

	case EZombieHitZone::Leg:
		Damage = LegDamage;
		break;
	}

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - Damage);
	const FVector ImpactPoint = ResolvedHit.bBlockingHit
		? ResolvedHit.ImpactPoint
		: GetActorLocation();

	ShowHitDebug(HitZone, ResolvedHit.BoneName);
	BP_OnZombieHit(
		HitZone,
		ResolvedHit.BoneName,
		Damage,
		ImpactPoint
	);

	if (HitZone == EZombieHitZone::Head || CurrentHealth <= 0.0f)
	{
		Die(HitZone, ResolvedHit, ImpactVelocity);
		return;
	}

	if (HitZone == EZombieHitZone::Torso)
	{
		BeginStagger();
	}
	else if (HitZone == EZombieHitZone::Leg)
	{
		EnterCrawling();
	}
}

EZombieHitZone AZombieCharacter::ClassifyHitBone(
	const FName BoneName
) const
{
	const FString Bone = BoneName.ToString().ToLower();

	if (Bone.Contains(TEXT("head"))
		|| Bone.Contains(TEXT("neck")))
	{
		return EZombieHitZone::Head;
	}

	if (Bone.Contains(TEXT("thigh"))
		|| Bone.Contains(TEXT("calf"))
		|| Bone.Contains(TEXT("foot"))
		|| Bone.Contains(TEXT("ball")))
	{
		return EZombieHitZone::Leg;
	}

	if (Bone.Contains(TEXT("arm"))
		|| Bone.Contains(TEXT("hand"))
		|| Bone.Contains(TEXT("clavicle")))
	{
		return EZombieHitZone::Arm;
	}

	// Pelvis, spine, and any future unclassified central body default here.
	return EZombieHitZone::Torso;
}

void AZombieCharacter::BeginStagger()
{
	if (PhysicalState == EZombiePhysicalState::Staggered
		|| PhysicalState == EZombiePhysicalState::Dead)
	{
		return;
	}

	StateBeforeStagger = PhysicalState;
	SetPhysicalState(EZombiePhysicalState::Staggered);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	GetWorldTimerManager().SetTimer(
		StaggerTimerHandle,
		this,
		&AZombieCharacter::EndStagger,
		TorsoStaggerDuration,
		false
	);
}

void AZombieCharacter::EndStagger()
{
	if (PhysicalState != EZombiePhysicalState::Staggered)
	{
		return;
	}

	SetPhysicalState(StateBeforeStagger);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
		Movement->MaxWalkSpeed =
			StateBeforeStagger == EZombiePhysicalState::Crawling
				? CrawlingMaxSpeed
				: StandingMaxWalkSpeed;
	}
}

void AZombieCharacter::EnterCrawling()
{
	if (PhysicalState == EZombiePhysicalState::Dead
		|| PhysicalState == EZombiePhysicalState::Crawling)
	{
		return;
	}

	if (PhysicalState == EZombiePhysicalState::Staggered)
	{
		StateBeforeStagger = EZombiePhysicalState::Crawling;
	}
	else
	{
		SetPhysicalState(EZombiePhysicalState::Crawling);
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = CrawlingMaxSpeed;
	}
}

void AZombieCharacter::Die(
	const EZombieHitZone FatalZone,
	const FHitResult& Hit,
	const FVector& ImpactVelocity
)
{
	if (PhysicalState == EZombiePhysicalState::Dead)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(StaggerTimerHandle);
	SetPhysicalState(EZombiePhysicalState::Dead);
	BP_OnZombieDeath(FatalZone, Hit.ImpactPoint);

	if (AKabulGameMode* GameMode =
		GetWorld() ? GetWorld()->GetAuthGameMode<AKabulGameMode>() : nullptr)
	{
		GameMode->NotifyZombieDefeated(this);
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (AController* ZombieController = GetController())
	{
		ZombieController->StopMovement();
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	USkeletalMeshComponent* ZombieMesh = GetMesh();

	if (!ZombieMesh)
	{
		return;
	}

	ZombieMesh->DetachFromComponent(
		FDetachmentTransformRules::KeepWorldTransform
	);
	ZombieMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	ZombieMesh->SetAllBodiesSimulatePhysics(true);
	ZombieMesh->SetSimulatePhysics(true);
	ZombieMesh->WakeAllRigidBodies();

	const FVector Impulse =
		ImpactVelocity.GetSafeNormal() * DeathImpulse;

	ZombieMesh->AddImpulseAtLocation(
		Impulse,
		Hit.ImpactPoint,
		Hit.BoneName
	);
}

void AZombieCharacter::ResetReactionState()
{
	GetWorldTimerManager().ClearTimer(StaggerTimerHandle);

	USkeletalMeshComponent* ZombieMesh = GetMesh();
	UCapsuleComponent* Capsule = GetCapsuleComponent();

	if (ZombieMesh)
	{
		ZombieMesh->SetSimulatePhysics(false);
		ZombieMesh->SetAllBodiesSimulatePhysics(false);
		ZombieMesh->SetPhysicsBlendWeight(0.0f);
		ZombieMesh->AttachToComponent(
			Capsule,
			FAttachmentTransformRules::KeepRelativeTransform
		);
		ZombieMesh->SetRelativeTransform(InitialMeshRelativeTransform);
		ZombieMesh->SetCollisionProfileName(InitialMeshCollisionProfile);
		ZombieMesh->SetCollisionEnabled(InitialMeshCollisionEnabled);
		ZombieMesh->SetCollisionResponseToChannel(
			ECC_GameTraceChannel1,
			ECR_Block
		);
	}

	if (Capsule)
	{
		Capsule->SetCollisionProfileName(InitialCapsuleCollisionProfile);
		Capsule->SetCollisionEnabled(InitialCapsuleCollisionEnabled);
		Capsule->SetCollisionResponseToChannel(
			ECC_GameTraceChannel1,
			ECR_Ignore
		);
	}

	SetActorTransform(
		InitialActorTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	CurrentHealth = MaxHealth;
	LastHitZone = EZombieHitZone::Torso;
	StateBeforeStagger = EZombiePhysicalState::Standing;
	SetPhysicalState(EZombiePhysicalState::Standing);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
		Movement->StopMovementImmediately();
		Movement->MaxWalkSpeed = StandingMaxWalkSpeed;
	}
}

void AZombieCharacter::SetPhysicalState(
	const EZombiePhysicalState NewState
)
{
	if (PhysicalState == NewState)
	{
		return;
	}

	const EZombiePhysicalState OldState = PhysicalState;
	PhysicalState = NewState;
	BP_OnPhysicalStateChanged(OldState, NewState);
}

void AZombieCharacter::ShowHitDebug(
	const EZombieHitZone HitZone,
	const FName BoneName
) const
{
	if (!bShowHitDebug || !GEngine)
	{
		return;
	}

#if !UE_BUILD_SHIPPING

	const FColor MessageColor =
		HitZone == EZombieHitZone::Head
			? FColor::Red
			: HitZone == EZombieHitZone::Leg
				? FColor::Yellow
				: HitZone == EZombieHitZone::Arm
					? FColor::Cyan
					: FColor::Orange;

	const FString ZoneName =
		StaticEnum<EZombieHitZone>()->GetDisplayNameTextByValue(
			static_cast<int64>(HitZone)
		).ToString();

	GEngine->AddOnScreenDebugMessage(
		-1,
		2.0f,
		MessageColor,
		FString::Printf(
			TEXT("Zombie hit: %s [%s] | Health: %.0f"),
			*ZoneName,
			*BoneName.ToString(),
			CurrentHealth
		)
	);
#endif
}
