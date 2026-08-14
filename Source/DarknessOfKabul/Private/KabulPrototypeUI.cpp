#include "KabulPrototypeUI.h"

#include "AudioDevice.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "KabulGameMode.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PlayerCharacter.h"
#include "Rendering/DrawElementTypes.h"
#include "Styling/CoreStyle.h"

namespace KabulUI
{
	constexpr TCHAR GameTitle[] = TEXT("DARKNESS OF KABUL");
	constexpr TCHAR BuildLabel[] = TEXT("PROTOTYPE BETA  |  BUILD 0.2.0");
	constexpr float VolumeValues[] = {1.0f, 0.75f, 0.5f, 0.25f, 0.0f};
	constexpr int32 VolumePercentages[] = {100, 75, 50, 25, 0};
	constexpr float SensitivityValues[] = {0.65f, 1.0f, 1.4f};
	constexpr const TCHAR* SensitivityLabels[] = {
		TEXT("LOW"), TEXT("NORMAL"), TEXT("HIGH")
	};
	constexpr float FieldOfViewValues[] = {80.0f, 90.0f, 105.0f};

	const FLinearColor Background(0.018f, 0.024f, 0.032f, 0.96f);
	const FLinearColor Panel(0.055f, 0.065f, 0.078f, 0.98f);
	const FLinearColor Text(0.88f, 0.91f, 0.94f, 1.0f);
	const FLinearColor Muted(0.52f, 0.58f, 0.64f, 1.0f);
	const FLinearColor Accent(0.19f, 0.91f, 0.62f, 1.0f);
	const FLinearColor Warning(1.0f, 0.56f, 0.13f, 1.0f);

	UTextBlock* MakeText(
		UWidgetTree* Tree,
		const FString& Value,
		const int32 Size,
		const FLinearColor& Color = Text,
		const bool bBold = false)
	{
		UTextBlock* Label = Tree->ConstructWidget<UTextBlock>();
		Label->SetText(FText::FromString(Value));
		Label->SetColorAndOpacity(Color);
		Label->SetJustification(ETextJustify::Center);
		Label->SetFont(FCoreStyle::GetDefaultFontStyle(
			bBold ? TEXT("Bold") : TEXT("Regular"),
			Size
		));
		return Label;
	}

	void AddToColumn(
		UVerticalBox* Column,
		UWidget* Widget,
		const FMargin& Padding = FMargin(0.0f, 4.0f),
		const EHorizontalAlignment Alignment = HAlign_Fill)
	{
		UVerticalBoxSlot* Slot = Column->AddChildToVerticalBox(Widget);
		Slot->SetPadding(Padding);
		Slot->SetHorizontalAlignment(Alignment);
	}

	void AddSpace(UWidgetTree* Tree, UVerticalBox* Column, const float Height)
	{
		USpacer* Spacer = Tree->ConstructWidget<USpacer>();
		Spacer->SetSize(FVector2D(1.0f, Height));
		AddToColumn(Column, Spacer, FMargin(0.0f));
	}

	UButton* AddButton(
		UWidgetTree* Tree,
		UVerticalBox* Column,
		const FString& Label,
		TObjectPtr<UTextBlock>* OutText = nullptr)
	{
		UButton* Button = Tree->ConstructWidget<UButton>();
		Button->SetBackgroundColor(Panel);

		UTextBlock* ButtonText = MakeText(Tree, Label, 18, Text, true);
		Button->AddChild(ButtonText);
		if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(ButtonText->Slot))
		{
			ButtonSlot->SetPadding(FMargin(32.0f, 11.0f));
		}
		AddToColumn(Column, Button, FMargin(0.0f, 5.0f));

		if (OutText)
		{
			*OutText = ButtonText;
		}

		return Button;
	}

	UBorder* MakeCenteredScreen(
		UWidgetTree* Tree,
		UCanvasPanel* Root,
		UVerticalBox*& OutColumn)
	{
		UBorder* Screen = Tree->ConstructWidget<UBorder>();
		Screen->SetBrushColor(Background);
		Screen->SetHorizontalAlignment(HAlign_Center);
		Screen->SetVerticalAlignment(VAlign_Center);

		UCanvasPanelSlot* ScreenSlot = Root->AddChildToCanvas(Screen);
		ScreenSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		ScreenSlot->SetOffsets(FMargin(0.0f));

		OutColumn = Tree->ConstructWidget<UVerticalBox>();
		Screen->SetContent(OutColumn);
		return Screen;
	}

	void AddIdentity(UWidgetTree* Tree, UVerticalBox* Column)
	{
		AddToColumn(
			Column,
			MakeText(Tree, GameTitle, 44, Accent, true),
			FMargin(0.0f, 2.0f)
		);
		AddToColumn(
			Column,
			MakeText(
				Tree,
				BuildLabel,
				12,
				Muted
			),
			FMargin(0.0f, 2.0f)
		);
	}
}

void UKabulPrototypeUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);

	if (GEngine && GEngine->GetGameUserSettings())
	{
		bFullscreen = GEngine->GetGameUserSettings()->GetFullscreenMode()
			!= EWindowMode::Windowed;
	}

	BuildInterface();
}

void UKabulPrototypeUI::NativeConstruct()
{
	Super::NativeConstruct();
	ApplySettings();
	ShowLoadingScreen();
}

void UKabulPrototypeUI::BuildInterface()
{
	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = RootCanvas;

	BuildLoadingPanel();
	BuildWelcomePanel();
	BuildMainMenuPanel();
	BuildSettingsPanel();
	BuildGameplayPanel();
	BuildPausePanel();
	BuildCompletionPanel();
}

void UKabulPrototypeUI::BuildLoadingPanel()
{
	UVerticalBox* Column = nullptr;
	LoadingPanel = KabulUI::MakeCenteredScreen(WidgetTree, RootCanvas, Column);
	KabulUI::AddIdentity(WidgetTree, Column);
	KabulUI::AddSpace(WidgetTree, Column, 34.0f);
	KabulUI::AddToColumn(
		Column,
		KabulUI::MakeText(
			WidgetTree,
			TEXT("LOADING..."),
			18,
			KabulUI::Text,
			true
		)
	);
}

void UKabulPrototypeUI::BuildWelcomePanel()
{
	UVerticalBox* Column = nullptr;
	WelcomePanel = KabulUI::MakeCenteredScreen(WidgetTree, RootCanvas, Column);
	KabulUI::AddIdentity(WidgetTree, Column);
	KabulUI::AddSpace(WidgetTree, Column, 42.0f);
	KabulUI::AddToColumn(
		Column,
		KabulUI::MakeText(
			WidgetTree,
			TEXT("PRESS ANY BUTTON TO CONTINUE"),
			20,
			KabulUI::Text,
			true
		)
	);
}

void UKabulPrototypeUI::BuildMainMenuPanel()
{
	UVerticalBox* Column = nullptr;
	MainMenuPanel = KabulUI::MakeCenteredScreen(WidgetTree, RootCanvas, Column);
	KabulUI::AddIdentity(WidgetTree, Column);
	KabulUI::AddSpace(WidgetTree, Column, 28.0f);

	UButton* NewGame = KabulUI::AddButton(WidgetTree, Column, TEXT("NEW GAME"));
	NewGame->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleNewGame);

	UButton* Continue = KabulUI::AddButton(WidgetTree, Column, TEXT("CONTINUE  [UNAVAILABLE]"));
	Continue->SetIsEnabled(false);

	UButton* Settings = KabulUI::AddButton(WidgetTree, Column, TEXT("SETTINGS"));
	Settings->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleSettings);

	UButton* Leave = KabulUI::AddButton(WidgetTree, Column, TEXT("LEAVE"));
	Leave->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleQuit);

}

void UKabulPrototypeUI::BuildSettingsPanel()
{
	UVerticalBox* Column = nullptr;
	SettingsPanel = KabulUI::MakeCenteredScreen(WidgetTree, RootCanvas, Column);
	KabulUI::AddIdentity(WidgetTree, Column);
	KabulUI::AddSpace(WidgetTree, Column, 22.0f);
	KabulUI::AddToColumn(
		Column,
		KabulUI::MakeText(WidgetTree, TEXT("SETTINGS"), 25, KabulUI::Text, true)
	);

	UButton* Volume = KabulUI::AddButton(
		WidgetTree,
		Column,
		TEXT("MASTER VOLUME: 100%"),
		&VolumeButtonText
	);
	Volume->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleVolume);

	UButton* Sensitivity = KabulUI::AddButton(
		WidgetTree,
		Column,
		TEXT("LOOK SENSITIVITY: NORMAL"),
		&SensitivityButtonText
	);
	Sensitivity->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleSensitivity);

	UButton* FieldOfView = KabulUI::AddButton(
		WidgetTree,
		Column,
		TEXT("FIELD OF VIEW: 90"),
		&FieldOfViewButtonText
	);
	FieldOfView->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleFieldOfView);

	UButton* Display = KabulUI::AddButton(
		WidgetTree,
		Column,
		bFullscreen ? TEXT("DISPLAY: FULLSCREEN") : TEXT("DISPLAY: WINDOWED"),
		&DisplayModeButtonText
	);
	Display->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleDisplayMode);

	UButton* Back = KabulUI::AddButton(WidgetTree, Column, TEXT("APPLY & BACK"));
	Back->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleSettingsBack);
}

void UKabulPrototypeUI::BuildGameplayPanel()
{
	UCanvasPanel* Panel = WidgetTree->ConstructWidget<UCanvasPanel>();
	GameplayPanel = Panel;
	Panel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	PanelSlot->SetOffsets(FMargin(0.0f));

	UTextBlock* Title = KabulUI::MakeText(
		WidgetTree,
		KabulUI::GameTitle,
		19,
		KabulUI::Accent,
		true
	);
	Title->SetJustification(ETextJustify::Left);
	UCanvasPanelSlot* TitleSlot = Panel->AddChildToCanvas(Title);
	TitleSlot->SetPosition(FVector2D(28.0f, 22.0f));
	TitleSlot->SetAutoSize(true);

	UTextBlock* Build = KabulUI::MakeText(
		WidgetTree,
		KabulUI::BuildLabel,
		10,
		KabulUI::Muted
	);
	Build->SetJustification(ETextJustify::Left);
	UCanvasPanelSlot* BuildSlot = Panel->AddChildToCanvas(Build);
	BuildSlot->SetPosition(FVector2D(29.0f, 48.0f));
	BuildSlot->SetAutoSize(true);

	ObjectiveText = KabulUI::MakeText(
		WidgetTree,
		TEXT("ZOMBIES 0/0   |   WOBBLE TARGETS 0/0"),
		14,
		KabulUI::Text,
		true
	);
	ObjectiveText->SetJustification(ETextJustify::Left);
	UCanvasPanelSlot* ObjectiveSlot = Panel->AddChildToCanvas(ObjectiveText);
	ObjectiveSlot->SetPosition(FVector2D(28.0f, 78.0f));
	ObjectiveSlot->SetAutoSize(true);

	UTextBlock* Controls = KabulUI::MakeText(
		WidgetTree,
		TEXT("RMB AIM  |  HOLD LMB PULL  |  RELEASE FIRE  |  R RESET  |  ESC PAUSE"),
		11,
		KabulUI::Muted
	);
	Controls->SetJustification(ETextJustify::Left);
	UCanvasPanelSlot* ControlsSlot = Panel->AddChildToCanvas(Controls);
	ControlsSlot->SetAnchors(FAnchors(0.0f, 1.0f));
	ControlsSlot->SetAlignment(FVector2D(0.0f, 1.0f));
	ControlsSlot->SetPosition(FVector2D(28.0f, -26.0f));
	ControlsSlot->SetAutoSize(true);

	ChargeStatusText = KabulUI::MakeText(
		WidgetTree,
		TEXT("HOLD LMB TO PULL"),
		13,
		KabulUI::Warning,
		true
	);
	UCanvasPanelSlot* StatusSlot = Panel->AddChildToCanvas(ChargeStatusText);
	StatusSlot->SetAnchors(FAnchors(0.5f, 1.0f));
	StatusSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	StatusSlot->SetPosition(FVector2D(0.0f, -67.0f));
	StatusSlot->SetAutoSize(true);

	ChargeBar = WidgetTree->ConstructWidget<UProgressBar>();
	ChargeBar->SetFillColorAndOpacity(KabulUI::Warning);
	UCanvasPanelSlot* ChargeSlot = Panel->AddChildToCanvas(ChargeBar);
	ChargeSlot->SetAnchors(FAnchors(0.5f, 1.0f));
	ChargeSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	ChargeSlot->SetPosition(FVector2D(0.0f, -46.0f));
	ChargeSlot->SetSize(FVector2D(260.0f, 8.0f));
}

void UKabulPrototypeUI::BuildPausePanel()
{
	UVerticalBox* Column = nullptr;
	PausePanel = KabulUI::MakeCenteredScreen(WidgetTree, RootCanvas, Column);
	KabulUI::AddIdentity(WidgetTree, Column);
	KabulUI::AddSpace(WidgetTree, Column, 24.0f);
	KabulUI::AddToColumn(
		Column,
		KabulUI::MakeText(WidgetTree, TEXT("PAUSED"), 28, KabulUI::Text, true)
	);

	UButton* Resume = KabulUI::AddButton(WidgetTree, Column, TEXT("RESUME"));
	Resume->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleResume);

	UButton* Restart = KabulUI::AddButton(WidgetTree, Column, TEXT("PLAY AGAIN"));
	Restart->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleNewGame);

	UButton* Settings = KabulUI::AddButton(WidgetTree, Column, TEXT("SETTINGS"));
	Settings->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleSettings);

	UButton* MainMenu = KabulUI::AddButton(WidgetTree, Column, TEXT("MAIN MENU"));
	MainMenu->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleMainMenu);

	UButton* Leave = KabulUI::AddButton(WidgetTree, Column, TEXT("LEAVE"));
	Leave->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleQuit);
}

void UKabulPrototypeUI::BuildCompletionPanel()
{
	UVerticalBox* Column = nullptr;
	CompletionPanel = KabulUI::MakeCenteredScreen(WidgetTree, RootCanvas, Column);
	KabulUI::AddIdentity(WidgetTree, Column);
	KabulUI::AddSpace(WidgetTree, Column, 36.0f);
	KabulUI::AddToColumn(
		Column,
		KabulUI::MakeText(
			WidgetTree,
			TEXT("THANKS FOR PLAYING"),
			32,
			KabulUI::Accent,
			true
		)
	);
	KabulUI::AddToColumn(
		Column,
		KabulUI::MakeText(
			WidgetTree,
			KabulUI::BuildLabel,
			17,
			KabulUI::Text,
			true
		)
	);
	KabulUI::AddSpace(WidgetTree, Column, 20.0f);

	UButton* PlayAgain = KabulUI::AddButton(WidgetTree, Column, TEXT("PLAY AGAIN"));
	PlayAgain->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleNewGame);

	UButton* Leave = KabulUI::AddButton(WidgetTree, Column, TEXT("LEAVE"));
	Leave->OnClicked.AddDynamic(this, &UKabulPrototypeUI::HandleQuit);
}

void UKabulPrototypeUI::ShowLoadingScreen()
{
	LoadingStartTime = FPlatformTime::Seconds();
	SetScreen(EPrototypeUIScreen::Loading);
}

void UKabulPrototypeUI::TogglePauseMenu()
{
	if (CurrentScreen == EPrototypeUIScreen::Gameplay)
	{
		SetScreen(EPrototypeUIScreen::Paused);
	}
	else if (CurrentScreen == EPrototypeUIScreen::Paused)
	{
		SetScreen(EPrototypeUIScreen::Gameplay);
	}
}

void UKabulPrototypeUI::ShowCompletionScreen()
{
	SetScreen(EPrototypeUIScreen::Completed);
}

void UKabulPrototypeUI::SetScreen(const EPrototypeUIScreen NewScreen)
{
	CurrentScreen = NewScreen;

	if (LoadingPanel)
	{
		const auto ShowOnly = [NewScreen](
			UWidget* Panel,
			const EPrototypeUIScreen PanelScreen,
			const ESlateVisibility VisibleState = ESlateVisibility::Visible)
		{
			Panel->SetVisibility(NewScreen == PanelScreen
				? VisibleState
				: ESlateVisibility::Collapsed);
		};

		ShowOnly(LoadingPanel, EPrototypeUIScreen::Loading);
		ShowOnly(WelcomePanel, EPrototypeUIScreen::Welcome);
		ShowOnly(MainMenuPanel, EPrototypeUIScreen::MainMenu);
		ShowOnly(SettingsPanel, EPrototypeUIScreen::Settings);
		ShowOnly(
			GameplayPanel,
			EPrototypeUIScreen::Gameplay,
			ESlateVisibility::SelfHitTestInvisible
		);
		ShowOnly(PausePanel, EPrototypeUIScreen::Paused);
		ShowOnly(CompletionPanel, EPrototypeUIScreen::Completed);
	}

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	const bool bGameplay = NewScreen == EPrototypeUIScreen::Gameplay;
	PlayerController->SetPause(!bGameplay);
	PlayerController->bShowMouseCursor = !bGameplay;

	if (bGameplay)
	{
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
	}
	else
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(
			EMouseLockMode::LockInFullscreen
		);
		PlayerController->SetInputMode(InputMode);
		SetKeyboardFocus();
	}
}

void UKabulPrototypeUI::NativeTick(
	const FGeometry& MyGeometry,
	const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (CurrentScreen == EPrototypeUIScreen::Loading
		&& FPlatformTime::Seconds() - LoadingStartTime >= 0.8)
	{
		SetScreen(EPrototypeUIScreen::Welcome);
		return;
	}

	if (CurrentScreen != EPrototypeUIScreen::Gameplay)
	{
		return;
	}

	UpdateGameplayReadout();
	PredictionRefreshRemaining -= InDeltaTime;

	APlayerCharacter* Player = GetPlayerCharacter();
	if (!Player || !Player->IsSlingshotAiming())
	{
		CachedTrajectory = FSlingshotTrajectoryPrediction();
		return;
	}

	if (PredictionRefreshRemaining <= 0.0f)
	{
		Player->GetSlingshotTrajectory(CachedTrajectory);
		PredictionRefreshRemaining = 0.05f;
	}
}

void UKabulPrototypeUI::UpdateGameplayReadout()
{
	APlayerCharacter* Player = GetPlayerCharacter();
	if (Player && ChargeBar && ChargeStatusText)
	{
		const bool bAiming = Player->IsSlingshotAiming();
		const bool bReady = Player->IsSlingshotShotReady();
		const bool bResetFeedback =
			Player->GetResetFeedbackTimeRemaining() > 0.0f;
		ChargeBar->SetVisibility(bAiming
			? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		ChargeStatusText->SetVisibility(bAiming || bResetFeedback
			? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		ChargeBar->SetPercent(Player->GetSlingshotPullAmount());
		ChargeBar->SetFillColorAndOpacity(
			bReady ? KabulUI::Accent : KabulUI::Warning
		);
		ChargeStatusText->SetColorAndOpacity(
			bReady ? KabulUI::Accent : KabulUI::Warning
		);
		ChargeStatusText->SetText(FText::FromString(
			bResetFeedback
				? TEXT("SCENARIO RESET")
				: (bReady ? TEXT("READY") : TEXT("HOLD LMB TO PULL"))
		));
	}

	if (ObjectiveText)
	{
		const AKabulGameMode* GameMode = GetWorld()
			? GetWorld()->GetAuthGameMode<AKabulGameMode>()
			: nullptr;
		if (GameMode)
		{
			ObjectiveText->SetText(FText::FromString(FString::Printf(
				TEXT("ZOMBIES %d/%d   |   WOBBLE TARGETS %d/%d"),
				GameMode->GetDefeatedZombieCount(),
				GameMode->GetZombieObjectiveCount(),
				GameMode->GetHitWobbleCount(),
				GameMode->GetWobbleObjectiveCount()
			)));
		}
	}
}

int32 UKabulPrototypeUI::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	const int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	const bool bParentEnabled) const
{
	const int32 BaseLayer = Super::NativePaint(
		Args,
		AllottedGeometry,
		MyCullingRect,
		OutDrawElements,
		LayerId,
		InWidgetStyle,
		bParentEnabled
	);

	if (CurrentScreen != EPrototypeUIScreen::Gameplay)
	{
		return BaseLayer;
	}

	const APlayerCharacter* Player = GetPlayerCharacter();
	const bool bReady = Player && Player->IsSlingshotShotReady();
	const FLinearColor ReticleColor = bReady
		? KabulUI::Accent
		: (Player && Player->IsSlingshotAiming()
			? KabulUI::Warning
			: KabulUI::Text);
	const FVector2D Center = AllottedGeometry.GetLocalSize() * 0.5f;
	const float Gap = 5.0f;
	const float Length = 8.0f;
	const int32 PaintLayer = BaseLayer + 1;

	DrawLine(OutDrawElements, AllottedGeometry, PaintLayer,
		Center + FVector2D(-Gap - Length, 0.0f),
		Center + FVector2D(-Gap, 0.0f), ReticleColor, 1.5f);
	DrawLine(OutDrawElements, AllottedGeometry, PaintLayer,
		Center + FVector2D(Gap, 0.0f),
		Center + FVector2D(Gap + Length, 0.0f), ReticleColor, 1.5f);
	DrawLine(OutDrawElements, AllottedGeometry, PaintLayer,
		Center + FVector2D(0.0f, -Gap - Length),
		Center + FVector2D(0.0f, -Gap), ReticleColor, 1.5f);
	DrawLine(OutDrawElements, AllottedGeometry, PaintLayer,
		Center + FVector2D(0.0f, Gap),
		Center + FVector2D(0.0f, Gap + Length), ReticleColor, 1.5f);

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController || !Player || !Player->IsSlingshotAiming())
	{
		return PaintLayer;
	}

	for (const FSlingshotTrajectoryImpact& Impact : CachedTrajectory.Impacts)
	{
		FVector2D ImpactScreen;
		if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			PlayerController,
			Impact.ImpactPoint,
			ImpactScreen,
			true))
		{
			continue;
		}

		const FLinearColor MarkerColor = Impact.bHitZombie
			? KabulUI::Accent
			: ReticleColor;

		if (!Impact.bHitZombie)
		{
			FVector2D IncomingScreen;
			if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
				PlayerController,
				Impact.IncomingGuidePoint,
				IncomingScreen,
				true))
			{
				DrawLine(OutDrawElements, AllottedGeometry, PaintLayer,
					IncomingScreen, ImpactScreen, MarkerColor, 2.4f);
			}

			if (Impact.bHasOutgoingSegment)
			{
				FVector2D OutgoingScreen;
				if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
					PlayerController,
					Impact.OutgoingGuidePoint,
					OutgoingScreen,
					true))
				{
					DrawLine(OutDrawElements, AllottedGeometry, PaintLayer,
						ImpactScreen, OutgoingScreen, MarkerColor, 2.4f);
				}
			}
		}

		DrawImpactMarker(
			OutDrawElements,
			AllottedGeometry,
			PaintLayer,
			ImpactScreen,
			MarkerColor,
			Impact.bHitZombie
		);
	}

	if (CachedTrajectory.Impacts.IsEmpty()
		&& !CachedTrajectory.FinalPoint.IsNearlyZero())
	{
		FVector2D FinalScreen;
		if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			PlayerController,
			CachedTrajectory.FinalPoint,
			FinalScreen,
			true))
		{
			DrawImpactMarker(OutDrawElements, AllottedGeometry, PaintLayer,
				FinalScreen, ReticleColor.CopyWithNewOpacity(0.55f), false);
		}
	}

	return PaintLayer;
}

void UKabulPrototypeUI::DrawLine(
	FSlateWindowElementList& OutDrawElements,
	const FGeometry& Geometry,
	const int32 LayerId,
	const FVector2D& Start,
	const FVector2D& End,
	const FLinearColor& Color,
	const float Thickness) const
{
	TArray<FVector2D> Points;
	Points.Reserve(2);
	Points.Add(Start);
	Points.Add(End);
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		Geometry.ToPaintGeometry(),
		Points,
		ESlateDrawEffect::None,
		Color,
		true,
		Thickness
	);
}

void UKabulPrototypeUI::DrawImpactMarker(
	FSlateWindowElementList& OutDrawElements,
	const FGeometry& Geometry,
	const int32 LayerId,
	const FVector2D& Position,
	const FLinearColor& Color,
	const bool bBodyTarget) const
{
	const float Size = bBodyTarget ? 10.0f : 6.0f;
	DrawLine(OutDrawElements, Geometry, LayerId,
		Position + FVector2D(-Size, -Size),
		Position + FVector2D(Size, Size), Color, 2.0f);
	DrawLine(OutDrawElements, Geometry, LayerId,
		Position + FVector2D(-Size, Size),
		Position + FVector2D(Size, -Size), Color, 2.0f);

	if (bBodyTarget)
	{
		const float Outer = Size + 5.0f;
		const float Corner = 5.0f;
		DrawLine(OutDrawElements, Geometry, LayerId,
			Position + FVector2D(-Outer, -Outer),
			Position + FVector2D(-Outer + Corner, -Outer), Color, 2.0f);
		DrawLine(OutDrawElements, Geometry, LayerId,
			Position + FVector2D(-Outer, -Outer),
			Position + FVector2D(-Outer, -Outer + Corner), Color, 2.0f);
		DrawLine(OutDrawElements, Geometry, LayerId,
			Position + FVector2D(Outer, Outer),
			Position + FVector2D(Outer - Corner, Outer), Color, 2.0f);
		DrawLine(OutDrawElements, Geometry, LayerId,
			Position + FVector2D(Outer, Outer),
			Position + FVector2D(Outer, Outer - Corner), Color, 2.0f);
	}
}

FReply UKabulPrototypeUI::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (CurrentScreen == EPrototypeUIScreen::Welcome)
	{
		SetScreen(EPrototypeUIScreen::MainMenu);
		return FReply::Handled();
	}

	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (CurrentScreen == EPrototypeUIScreen::Paused)
		{
			SetScreen(EPrototypeUIScreen::Gameplay);
			return FReply::Handled();
		}
		if (CurrentScreen == EPrototypeUIScreen::Settings)
		{
			HandleSettingsBack();
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UKabulPrototypeUI::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (CurrentScreen == EPrototypeUIScreen::Welcome)
	{
		SetScreen(EPrototypeUIScreen::MainMenu);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

APlayerCharacter* UKabulPrototypeUI::GetPlayerCharacter() const
{
	return GetOwningPlayer()
		? Cast<APlayerCharacter>(GetOwningPlayer()->GetPawn())
		: nullptr;
}

void UKabulPrototypeUI::HandleNewGame()
{
	if (APlayerCharacter* Player = GetPlayerCharacter())
	{
		Player->RestartPrototype();
	}
	SetScreen(EPrototypeUIScreen::Gameplay);
}

void UKabulPrototypeUI::HandleSettings()
{
	SettingsReturnScreen = CurrentScreen == EPrototypeUIScreen::Paused
		? EPrototypeUIScreen::Paused
		: EPrototypeUIScreen::MainMenu;
	SetScreen(EPrototypeUIScreen::Settings);
}

void UKabulPrototypeUI::HandleQuit()
{
	UKismetSystemLibrary::QuitGame(
		this,
		GetOwningPlayer(),
		EQuitPreference::Quit,
		false
	);
}

void UKabulPrototypeUI::HandleResume()
{
	SetScreen(EPrototypeUIScreen::Gameplay);
}

void UKabulPrototypeUI::HandleMainMenu()
{
	SetScreen(EPrototypeUIScreen::MainMenu);
}

void UKabulPrototypeUI::HandleSettingsBack()
{
	ApplySettings();
	SetScreen(SettingsReturnScreen);
}

void UKabulPrototypeUI::HandleVolume()
{
	VolumeIndex = (VolumeIndex + 1)
		% UE_ARRAY_COUNT(KabulUI::VolumePercentages);
	VolumeButtonText->SetText(FText::FromString(FString::Printf(
		TEXT("MASTER VOLUME: %d%%"),
		KabulUI::VolumePercentages[VolumeIndex]
	)));
	ApplySettings();
}

void UKabulPrototypeUI::HandleSensitivity()
{
	SensitivityIndex = (SensitivityIndex + 1)
		% UE_ARRAY_COUNT(KabulUI::SensitivityLabels);
	SensitivityButtonText->SetText(FText::FromString(FString::Printf(
		TEXT("LOOK SENSITIVITY: %s"),
		KabulUI::SensitivityLabels[SensitivityIndex]
	)));
	ApplySettings();
}

void UKabulPrototypeUI::HandleFieldOfView()
{
	FieldOfViewIndex = (FieldOfViewIndex + 1)
		% UE_ARRAY_COUNT(KabulUI::FieldOfViewValues);
	FieldOfViewButtonText->SetText(FText::FromString(FString::Printf(
		TEXT("FIELD OF VIEW: %.0f"),
		KabulUI::FieldOfViewValues[FieldOfViewIndex]
	)));
	ApplySettings();
}

void UKabulPrototypeUI::HandleDisplayMode()
{
	bFullscreen = !bFullscreen;
	DisplayModeButtonText->SetText(FText::FromString(
		bFullscreen ? TEXT("DISPLAY: FULLSCREEN") : TEXT("DISPLAY: WINDOWED")
	));
}

void UKabulPrototypeUI::ApplySettings()
{
	if (GEngine)
	{
		if (FAudioDeviceHandle AudioDevice = GEngine->GetMainAudioDevice())
		{
			AudioDevice->SetTransientPrimaryVolume(
				KabulUI::VolumeValues[VolumeIndex]
			);
		}

		if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
		{
			Settings->SetFullscreenMode(
				bFullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed
			);
			Settings->ApplySettings(false);
			Settings->SaveSettings();
		}
	}

	if (APlayerCharacter* Player = GetPlayerCharacter())
	{
		Player->SetLookSensitivity(
			KabulUI::SensitivityValues[SensitivityIndex]
		);
		Player->SetFirstPersonFieldOfView(
			KabulUI::FieldOfViewValues[FieldOfViewIndex]
		);
	}
}
