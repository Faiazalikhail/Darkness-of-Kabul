#include "Objectives/DOKEscortController.h"

#include "Engine/World.h"
#include "Game/DOKGameMode.h"

ADOKEscortController::ADOKEscortController()
{
	PrimaryActorTick.bCanEverTick = false;

	StageObjectives = {
		FText::FromString(TEXT("Practice with the slingshot and save the dogs.")),
		FText::FromString(TEXT("Clear the irrigation field.")),
		FText::FromString(TEXT("Protect the flock at the village entrance.")),
		FText::FromString(TEXT("Reach the home gate.")),
		FText::FromString(TEXT("Defend the courtyard and close the gate."))
	};
}

void ADOKEscortController::BeginPlay()
{
	Super::BeginPlay();
	OnRouteStageChanged.Broadcast(CurrentStage, GetCurrentObjective());
	OnFlockDangerChanged.Broadcast(GetDangerPercent());
}

bool ADOKEscortController::AdvanceStage()
{
	if (bEscortComplete || StageObjectives.IsEmpty())
	{
		return false;
	}

	if (CurrentStage + 1 >= StageObjectives.Num())
	{
		MarkFlockHome();
		return true;
	}

	++CurrentStage;
	OnRouteStageChanged.Broadcast(CurrentStage, GetCurrentObjective());
	return true;
}

void ADOKEscortController::AddFlockDanger(const float Amount)
{
	if (bEscortComplete || Amount <= 0.0f)
	{
		return;
	}

	CurrentFlockDanger = FMath::Clamp(CurrentFlockDanger + Amount, 0.0f, MaximumFlockDanger);
	OnFlockDangerChanged.Broadcast(GetDangerPercent());

	if (CurrentFlockDanger >= MaximumFlockDanger)
	{
		if (ADOKGameMode* GameMode = GetWorld()->GetAuthGameMode<ADOKGameMode>())
		{
			GameMode->LoseGame(TEXT("The flock was overwhelmed."));
		}
	}
}

void ADOKEscortController::ReduceFlockDanger(const float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	CurrentFlockDanger = FMath::Clamp(CurrentFlockDanger - Amount, 0.0f, MaximumFlockDanger);
	OnFlockDangerChanged.Broadcast(GetDangerPercent());
}

void ADOKEscortController::MarkFlockHome()
{
	if (bEscortComplete)
	{
		return;
	}

	bEscortComplete = true;
	OnEscortCompleted.Broadcast();

	if (ADOKGameMode* GameMode = GetWorld()->GetAuthGameMode<ADOKGameMode>())
	{
		GameMode->WinGame();
	}
}

float ADOKEscortController::GetDangerPercent() const
{
	return MaximumFlockDanger > 0.0f ? CurrentFlockDanger / MaximumFlockDanger : 0.0f;
}

FText ADOKEscortController::GetCurrentObjective() const
{
	return StageObjectives.IsValidIndex(CurrentStage)
		? StageObjectives[CurrentStage]
		: FText::GetEmpty();
}
