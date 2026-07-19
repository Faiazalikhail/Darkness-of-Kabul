#include "Enemies/DOKZombie.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ADOKZombie::ADOKZombie()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 240.0f, 0.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
}

void ADOKZombie::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaximumHealth;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	if (!TargetActor)
	{
		TargetActor = UGameplayStatics::GetPlayerPawn(this, 0);
	}
	SetZombieState(TargetActor ? EDOKZombieState::Walk : EDOKZombieState::Idle);
}

void ADOKZombie::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EDOKZombieState::Walk)
	{
		UpdateMovement();
		TryAttack();
	}
	else if (CurrentState == EDOKZombieState::Attack)
	{
		TryAttack();
	}
}

float ADOKZombie::TakeDamage(
	const float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (AppliedDamage <= 0.0f || CurrentState == EDOKZombieState::Dead)
	{
		return 0.0f;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - AppliedDamage, 0.0f, MaximumHealth);
	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
	else if (AppliedDamage >= BodyHitStaggerThreshold)
	{
		SetZombieState(EDOKZombieState::Stagger);
		GetWorldTimerManager().SetTimer(
			StaggerTimer,
			this,
			&ADOKZombie::RecoverFromStagger,
			StaggerDuration,
			false);
	}

	return AppliedDamage;
}

void ADOKZombie::SetTargetActor(AActor* NewTarget)
{
	TargetActor = NewTarget;
	if (TargetActor && CurrentState == EDOKZombieState::Idle)
	{
		SetZombieState(EDOKZombieState::Walk);
	}
}

float ADOKZombie::GetHealthPercent() const
{
	return MaximumHealth > 0.0f ? CurrentHealth / MaximumHealth : 0.0f;
}

void ADOKZombie::SetZombieState(const EDOKZombieState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;
	OnZombieStateChanged.Broadcast(CurrentState);
}

void ADOKZombie::UpdateMovement()
{
	if (!TargetActor)
	{
		SetZombieState(EDOKZombieState::Idle);
		return;
	}

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->MoveToActor(TargetActor, AttackRange * 0.8f, true, true, true, nullptr, true);
	}
}

void ADOKZombie::TryAttack()
{
	if (!TargetActor || !GetWorld())
	{
		return;
	}

	const float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
	if (Distance > AttackRange)
	{
		if (CurrentState == EDOKZombieState::Attack)
		{
			SetZombieState(EDOKZombieState::Walk);
		}
		return;
	}

	SetZombieState(EDOKZombieState::Attack);
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime >= AttackCooldown)
	{
		UGameplayStatics::ApplyDamage(TargetActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
		LastAttackTime = CurrentTime;
	}
}

void ADOKZombie::RecoverFromStagger()
{
	if (CurrentState != EDOKZombieState::Dead)
	{
		SetZombieState(TargetActor ? EDOKZombieState::Walk : EDOKZombieState::Idle);
	}
}

void ADOKZombie::Die()
{
	SetZombieState(EDOKZombieState::Dead);
	GetWorldTimerManager().ClearTimer(StaggerTimer);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(5.0f);
}
