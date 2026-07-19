#include "Game/DOKGameMode.h"

#include "Character/DOKCharacter.h"
#include "Game/DOKPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/DOKHUDWidget.h"

ADOKGameMode::ADOKGameMode()
{
	DefaultPawnClass = ADOKCharacter::StaticClass();
	PlayerControllerClass = ADOKPlayerController::StaticClass();
}

void ADOKGameMode::BeginPlay()
{
	Super::BeginPlay();
	CurrentState = EDOKGameState::Playing;
	SetObjective(FText::FromString(TEXT("Practice with the slingshot and save the dogs.")));

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UDOKHUDWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
}

void ADOKGameMode::WinGame()
{
	if (CurrentState != EDOKGameState::Playing)
	{
		return;
	}

	CurrentState = EDOKGameState::Won;
	OnGameStateChanged.Broadcast(CurrentState, FText::FromString(TEXT("The flock is safely home.")));
}

void ADOKGameMode::LoseGame(const FString& Reason)
{
	if (CurrentState != EDOKGameState::Playing)
	{
		return;
	}

	CurrentState = EDOKGameState::Lost;
	OnGameStateChanged.Broadcast(CurrentState, FText::FromString(Reason));
}

void ADOKGameMode::RestartCurrentLevel()
{
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(this, FName(*World->GetName()), false);
	}
}

void ADOKGameMode::SetObjective(const FText& NewObjective)
{
	CurrentObjective = NewObjective;
	OnObjectiveChanged.Broadcast(CurrentObjective);
}
