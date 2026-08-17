#include "KabulGameMode.h"

#include "KabulHUD.h"
#include "KabulPlayerController.h"
#include "PlayerCharacter.h"
#include "ZombieCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

AKabulGameMode::AKabulGameMode()
{
	// The game starts with the movement character. A Blueprint child
	// may replace visuals later without replacing the locomotion code.
	DefaultPawnClass = APlayerCharacter::StaticClass();
	PlayerControllerClass = AKabulPlayerController::StaticClass();
	HUDClass = AKabulHUD::StaticClass();

	// Objective actors are discovered in BeginPlay so the level can change
	// without hard-coded counts.
}

void AKabulGameMode::BeginPlay()
{
	Super::BeginPlay();

	ZombieObjectives.Reset();
	WobbleObjectives.Reset();
	WobbleStartTransforms.Reset();

	for (TActorIterator<AZombieCharacter> Zombie(GetWorld()); Zombie; ++Zombie)
	{
		ZombieObjectives.Add(*Zombie);
	}

	for (TActorIterator<AActor> Actor(GetWorld()); Actor; ++Actor)
	{
		if (Actor->GetClass()->GetName().Contains(TEXT("WobbleTarget")))
		{
			WobbleObjectives.Add(*Actor);
			WobbleStartTransforms.Add(*Actor, Actor->GetActorTransform());
		}
	}
}

void AKabulGameMode::NotifyZombieDefeated(AZombieCharacter* Zombie)
{
	if (Zombie && ZombieObjectives.Contains(Zombie))
	{
		DefeatedZombies.Add(Zombie);
		CheckForPrototypeCompletion();
	}
}

void AKabulGameMode::NotifyWobbleTargetHit(AActor* WobbleTarget)
{
	if (WobbleTarget && WobbleObjectives.Contains(WobbleTarget))
	{
		HitWobbleTargets.Add(WobbleTarget);
		CheckForPrototypeCompletion();
	}
}

void AKabulGameMode::ResetPrototypeObjectives()
{
	DefeatedZombies.Reset();
	HitWobbleTargets.Reset();
	bCompletionShown = false;

	for (const TPair<TWeakObjectPtr<AActor>, FTransform>& Entry
		: WobbleStartTransforms)
	{
		AActor* WobbleTarget = Entry.Key.Get();

		if (!WobbleTarget)
		{
			continue;
		}

		// Wobble targets are static props. Restoring a static root logs a
		// mobility warning and moves nothing, so only restore what can move.
		const USceneComponent* Root = WobbleTarget->GetRootComponent();

		if (Root && Root->Mobility == EComponentMobility::Movable)
		{
			WobbleTarget->SetActorTransform(
				Entry.Value,
				false,
				nullptr,
				ETeleportType::TeleportPhysics
			);
		}
	}
}

void AKabulGameMode::CheckForPrototypeCompletion()
{
	if (bCompletionShown)
	{
		return;
	}

	const int32 TotalObjectiveCount =
		ZombieObjectives.Num() + WobbleObjectives.Num();
	const bool bAllZombiesDefeated =
		DefeatedZombies.Num() >= ZombieObjectives.Num();
	const bool bAllWobbleTargetsHit =
		HitWobbleTargets.Num() >= WobbleObjectives.Num();

	if (TotalObjectiveCount <= 0
		|| !bAllZombiesDefeated
		|| !bAllWobbleTargetsHit)
	{
		return;
	}

	bCompletionShown = true;

	if (APlayerController* PlayerController =
		GetWorld()->GetFirstPlayerController())
	{
		if (APlayerCharacter* Player =
			Cast<APlayerCharacter>(PlayerController->GetPawn()))
		{
			Player->ShowPrototypeCompletion();
		}
	}
}
