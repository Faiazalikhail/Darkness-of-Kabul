#include "ZombieCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"
#include "KabulGameMode.h"
#include "PlayerCharacter.h"
#include "TimerManager.h"

#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));

	// Placed zombies must possess an AI controller so navigation can drive them.
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

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

	Health->InitializeHealth(MaxHealth);
	CurrentHealth = Health->GetCurrentHealth();
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

	// Sensing runs on its own timer so many placed zombies stay cheap. The
	// random first delay spreads the cost across frames instead of spiking.
	GetWorldTimerManager().SetTimer(
		AISenseTimerHandle,
		this,
		&AZombieCharacter::UpdateAI,
		FMath::Max(0.02f, SenseInterval),
		true,
		FMath::FRandRange(0.0f, SenseInterval)
	);
}

APawn* AZombieCharacter::FindPlayerPawn() const
{
	const UWorld* World = GetWorld();
	APlayerController* PlayerController =
		World ? World->GetFirstPlayerController() : nullptr;

	return PlayerController ? PlayerController->GetPawn() : nullptr;
}

bool AZombieCharacter::IsChasing() const
{
	return bAlerted
		&& PhysicalState != EZombiePhysicalState::Dead
		&& PhysicalState != EZombiePhysicalState::Staggered;
}

void AZombieCharacter::RefreshMovementSpeed()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();

	if (!Movement)
	{
		return;
	}

	if (PhysicalState == EZombiePhysicalState::Crawling)
	{
		Movement->MaxWalkSpeed = CrawlingMaxSpeed;
		return;
	}

	// Alerted zombies close distance quickly; idle ones keep the placed speed.
	Movement->MaxWalkSpeed = bAlerted
		? ChaseMaxWalkSpeed
		: StandingMaxWalkSpeed;
}

bool AZombieCharacter::CanSeePlayer(const APawn* PlayerPawn) const
{
	const UWorld* World = GetWorld();

	if (!World || !PlayerPawn)
	{
		return false;
	}

	const FVector EyeLocation = GetActorLocation();
	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	const FVector ToPlayer = PlayerLocation - EyeLocation;

	if (ToPlayer.SizeSquared() > FMath::Square(SightRange))
	{
		return false;
	}

	// Forward vision cone. A zombie cannot notice the player behind its back.
	const FVector DirectionToPlayer = ToPlayer.GetSafeNormal();
	const float CosineLimit =
		FMath::Cos(FMath::DegreesToRadians(SightHalfAngleDegrees));

	if (FVector::DotProduct(GetActorForwardVector(), DirectionToPlayer)
		< CosineLimit)
	{
		return false;
	}

	// Walls block sight, so the player can break line of sight to stay hidden.
	FCollisionQueryParams QueryParameters(
		SCENE_QUERY_STAT(ZombieSight),
		false,
		this
	);
	QueryParameters.AddIgnoredActor(PlayerPawn);

	FHitResult Blocking;
	const bool bBlocked = World->LineTraceSingleByChannel(
		Blocking,
		EyeLocation,
		PlayerLocation,
		ECC_Visibility,
		QueryParameters
	);

	return !bBlocked;
}

void AZombieCharacter::AlertToPlayer(APawn* PlayerPawn)
{
	if (PhysicalState == EZombiePhysicalState::Dead || !PlayerPawn)
	{
		return;
	}

	ChaseTarget = PlayerPawn;

	if (bAlerted)
	{
		return;
	}

	bAlerted = true;
	RefreshMovementSpeed();
	BP_OnZombieAlerted();
}

void AZombieCharacter::RaiseDisturbance(APawn* PlayerPawn)
{
	UWorld* World = GetWorld();

	if (!World || !PlayerPawn)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	const float RadiusSquared = FMath::Square(DisturbanceRadius);

	// Only the local group reacts. Zombies beyond the radius stay unaware, so
	// striking one of four pulls its neighbours and not the whole level.
	for (TActorIterator<AZombieCharacter> It(World); It; ++It)
	{
		AZombieCharacter* Other = *It;

		if (!Other || Other == this)
		{
			continue;
		}

		if (FVector::DistSquared(Origin, Other->GetActorLocation())
			<= RadiusSquared)
		{
			Other->AlertToPlayer(PlayerPawn);
		}
	}
}

void AZombieCharacter::UpdateAI()
{
	UWorld* World = GetWorld();

	if (!World || PhysicalState == EZombiePhysicalState::Dead)
	{
		return;
	}

	APawn* PlayerPawn = ChaseTarget.Get();

	if (!PlayerPawn)
	{
		PlayerPawn = FindPlayerPawn();
	}

	APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerPawn);

	// A dead player ends the chase so bodies do not keep swinging at a corpse.
	if (!PlayerPawn || (Player && Player->IsPlayerDead()))
	{
		if (AController* ZombieController = GetController())
		{
			ZombieController->StopMovement();
		}

		return;
	}

	if (!bAlerted)
	{
		if (!CanSeePlayer(PlayerPawn))
		{
			return;
		}

		AlertToPlayer(PlayerPawn);
	}
	else if (FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation())
		> LoseInterestDistance)
	{
		// The player has broken away far enough. Give up and go back to
		// unaware so the zombie can be surprised again later.
		bAlerted = false;
		ChaseTarget = nullptr;
		RefreshMovementSpeed();

		if (AController* ZombieController = GetController())
		{
			ZombieController->StopMovement();
		}

		return;
	}

	// A staggered zombie is briefly unable to advance or swing.
	if (PhysicalState == EZombiePhysicalState::Staggered)
	{
		if (AController* ZombieController = GetController())
		{
			ZombieController->StopMovement();
		}

		return;
	}

	const float Distance = FVector::Dist(
		GetActorLocation(),
		PlayerPawn->GetActorLocation()
	);

	if (Distance > AttackRange)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToActor(
			GetController(),
			PlayerPawn
		);

		return;
	}

	// In range: stop closing and swing on the attack interval.
	if (AController* ZombieController = GetController())
	{
		ZombieController->StopMovement();
	}

	const float Now = World->GetTimeSeconds();

	if (Now - LastAttackTime < AttackInterval)
	{
		return;
	}

	LastAttackTime = Now;
	BP_OnZombieAttack(PlayerPawn->GetActorLocation());

	if (Player)
	{
		Player->ApplyZombieDamage(AttackDamage);
	}
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

	// A head hit is lethal by rule rather than by damage value, so it empties
	// the pool regardless of how much health is left.
	const float Damage = HitZone == EZombieHitZone::Head
		? Health->ApplyLethalDamage()
		: Health->ApplyDamage(
			HitZone == EZombieHitZone::Torso ? TorsoDamage
			: HitZone == EZombieHitZone::Arm ? ArmDamage
			: LegDamage
		);

	CurrentHealth = Health->GetCurrentHealth();
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

	// Being struck is a disturbance: this zombie and its nearby group wake up
	// and converge, even when the stone arrived from outside their vision.
	if (APawn* PlayerPawn = FindPlayerPawn())
	{
		AlertToPlayer(PlayerPawn);
		RaiseDisturbance(PlayerPawn);
	}

	if (HitZone == EZombieHitZone::Head || Health->IsDepleted())
	{
		Die(HitZone, ResolvedHit, ImpactVelocity);
		return;
	}

	if (HitZone == EZombieHitZone::Torso)
	{
		// A downed zombie stays down. Only a zombie still on its feet can be
		// staggered, otherwise it would stand up to play the stagger reaction.
		if (PhysicalState != EZombiePhysicalState::Crawling)
		{
			BeginStagger();
		}
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
	}

	// Restores crawl speed, chase speed, or the placed walk speed as applicable.
	RefreshMovementSpeed();
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

	Health->ResetHealth();
	CurrentHealth = Health->GetCurrentHealth();
	LastHitZone = EZombieHitZone::Torso;
	StateBeforeStagger = EZombiePhysicalState::Standing;
	SetPhysicalState(EZombiePhysicalState::Standing);

	// Reset returns every zombie to unaware so the scenario restarts calmly.
	bAlerted = false;
	ChaseTarget = nullptr;
	LastAttackTime = -1000.0f;

	if (AController* ZombieController = GetController())
	{
		ZombieController->StopMovement();
	}

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
