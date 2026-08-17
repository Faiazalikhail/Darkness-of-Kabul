#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlingshotTrajectoryTypes.h"
#include "KabulPrototypeUI.generated.h"

class UButton;
class UCanvasPanel;
class UProgressBar;
class UTextBlock;

enum class EPrototypeUIScreen : uint8
{
	Loading,
	Welcome,
	MainMenu,
	Settings,
	Gameplay,
	Paused,
	Completed,
	GameOver,
	ConfirmReset
};

/** Complete native UMG presentation for the target-practice prototype. */
UCLASS()
class DARKNESSOFKABUL_API UKabulPrototypeUI : public UUserWidget
{
	GENERATED_BODY()

public:
	void TogglePauseMenu();
	void ShowCompletionScreen();
	void ShowGameOverScreen();
	void ShowResetConfirmScreen();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime
	) override;
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled
	) const override;
	virtual FReply NativeOnKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent
	) override;
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent
	) override;

private:
	void BuildInterface();
	void BuildLoadingPanel();
	void BuildWelcomePanel();
	void BuildMainMenuPanel();
	void BuildSettingsPanel();
	void BuildGameplayPanel();
	void BuildPausePanel();
	void BuildCompletionPanel();
	void BuildGameOverPanel();
	void BuildResetConfirmPanel();

	UFUNCTION()
	void HandleConfirmReset();

	UFUNCTION()
	void HandleCancelReset();
	void ShowLoadingScreen();
	void SetScreen(EPrototypeUIScreen NewScreen);
	void ApplySettings();
	class APlayerCharacter* GetPlayerCharacter() const;
	void UpdateGameplayReadout();
	void DrawLine(
		FSlateWindowElementList& OutDrawElements,
		const FGeometry& Geometry,
		int32 LayerId,
		const FVector2D& Start,
		const FVector2D& End,
		const FLinearColor& Color,
		float Thickness
	) const;
	void DrawImpactMarker(
		FSlateWindowElementList& OutDrawElements,
		const FGeometry& Geometry,
		int32 LayerId,
		const FVector2D& Position,
		const FLinearColor& Color,
		bool bBodyTarget
	) const;

	UFUNCTION()
	void HandleNewGame();

	UFUNCTION()
	void HandleSettings();

	UFUNCTION()
	void HandleQuit();

	UFUNCTION()
	void HandleResume();

	UFUNCTION()
	void HandleMainMenu();

	UFUNCTION()
	void HandleSettingsBack();

	UFUNCTION()
	void HandleVolume();

	UFUNCTION()
	void HandleSensitivity();

	UFUNCTION()
	void HandleFieldOfView();

	UFUNCTION()
	void HandleDisplayMode();

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> LoadingPanel;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> WelcomePanel;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> MainMenuPanel;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> SettingsPanel;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> GameplayPanel;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> PausePanel;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> CompletionPanel;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> GameOverPanel;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> ResetConfirmPanel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ObjectiveText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ChargeStatusText;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> ChargeBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> VolumeButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SensitivityButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FieldOfViewButtonText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DisplayModeButtonText;

	EPrototypeUIScreen CurrentScreen = EPrototypeUIScreen::Loading;
	EPrototypeUIScreen SettingsReturnScreen = EPrototypeUIScreen::MainMenu;
	FSlingshotTrajectoryPrediction CachedTrajectory;
	double LoadingStartTime = 0.0;
	float PredictionRefreshRemaining = 0.0f;
	int32 VolumeIndex = 0;
	int32 SensitivityIndex = 1;
	int32 FieldOfViewIndex = 1;
	bool bFullscreen = true;
};
