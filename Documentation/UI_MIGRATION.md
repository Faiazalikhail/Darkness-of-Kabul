# UI Migration — moving screen layout from C++ into UMG assets

## Why

`KabulPrototypeUI.cpp` builds every screen by hand in C++. Roughly 700 of its
lines are widget construction. That is not how Unreal UI is normally authored:
production projects build widgets as UMG assets and keep C++ for behaviour.
Moving the layout removes the largest block of machine-shaped code in the
project and puts the presentation where a reviewer expects to find it.

## What moves and what stays

| Stays in C++ | Moves to UMG |
|---|---|
| Screen switching (`SetScreen`) | Panel layout and styling |
| Per-frame readouts (`NativeTick`) | Text blocks, buttons, bars |
| Reticle and impact markers (`NativePaint`) | Fonts, colours, spacing |
| Button responses (`Handle*`) | Anchors and alignment |
| Settings load/save/apply | — |

**The marker painting must stay in C++.** It projects world-space impact points
onto the screen every frame; that is runtime rendering, not layout.

## How binding works

C++ declares what it needs:

```cpp
UPROPERTY(meta = (BindWidget))
TObjectPtr<UTextBlock> HealthText;
```

You create a widget with that exact name in the UMG asset. Unreal connects them
when the Blueprint compiles, and reports a compile error if a name is missing or
the type is wrong. Mistakes surface immediately rather than at runtime.

Use `BindWidgetOptional` instead while a screen is only partly built.

## Recommended order

Do the **main menu first**, on its own. It is four buttons with no per-frame
logic, so it proves the whole approach in about twenty minutes. Once it works,
the remaining screens are mechanical repetition.

## Naming contract

Every name below must match exactly, including case.

### Panels — one container per screen

| Widget name | Type |
|---|---|
| `LoadingPanel` | any panel widget |
| `WelcomePanel` | any panel widget |
| `MainMenuPanel` | any panel widget |
| `SettingsPanel` | any panel widget |
| `GameplayPanel` | any panel widget |
| `PausePanel` | any panel widget |
| `CompletionPanel` | any panel widget |
| `GameOverPanel` | any panel widget |
| `ResetConfirmPanel` | any panel widget |

### Readouts updated every frame

| Widget name | Type | Shows |
|---|---|---|
| `HealthText` | TextBlock | `HEALTH 100%` |
| `ObjectiveText` | TextBlock | zombie and wobble counts |
| `ChargeStatusText` | TextBlock | `READY` / `HOLD LMB TO PULL` / `SCENARIO RESET` |
| `ChargeBar` | ProgressBar | pull amount 0-1 |

### Settings labels updated on click

| Widget name | Type |
|---|---|
| `VolumeButtonText` | TextBlock |
| `SensitivityButtonText` | TextBlock |
| `FieldOfViewButtonText` | TextBlock |
| `DisplayModeButtonText` | TextBlock |

### Buttons and the handler each one calls

| Screen | Widget name | Calls |
|---|---|---|
| Main menu | `NewGameButton` | `HandleNewGame` |
| Main menu | `ContinueButton` | none — set **Is Enabled** off |
| Main menu | `SettingsButton` | `HandleSettings` |
| Main menu | `LeaveButton` | `HandleQuit` |
| Settings | `VolumeButton` | `HandleVolume` |
| Settings | `SensitivityButton` | `HandleSensitivity` |
| Settings | `FieldOfViewButton` | `HandleFieldOfView` |
| Settings | `DisplayModeButton` | `HandleDisplayMode` |
| Settings | `SettingsBackButton` | `HandleSettingsBack` |
| Pause | `ResumeButton` | `HandleResume` |
| Pause | `PausePlayAgainButton` | `HandleNewGame` |
| Pause | `PauseSettingsButton` | `HandleSettings` |
| Pause | `MainMenuButton` | `HandleMainMenu` |
| Pause | `PauseLeaveButton` | `HandleQuit` |
| Completion | `CompletionPlayAgainButton` | `HandleNewGame` |
| Completion | `CompletionLeaveButton` | `HandleQuit` |
| Game over | `GameOverTryAgainButton` | `HandleNewGame` |
| Game over | `GameOverLeaveButton` | `HandleQuit` |
| Reset prompt | `ConfirmResetButton` | `HandleConfirmReset` |
| Reset prompt | `CancelResetButton` | `HandleCancelReset` |

Several buttons deliberately share `HandleNewGame`; restarting is the same
action wherever it is offered.

## Style values to reproduce

Taken from the current C++ so the migrated screens match.

| Role | RGBA |
|---|---|
| Background | 0.018, 0.024, 0.032, 0.96 |
| Panel / button fill | 0.055, 0.065, 0.078, 0.98 |
| Text | 0.88, 0.91, 0.94 |
| Muted | 0.52, 0.58, 0.64 |
| Accent (green) | 0.19, 0.91, 0.62 |
| Warning (orange) | 1.0, 0.56, 0.13 |

Title 44 bold, build label 12 muted, menu buttons 18 bold with 32 x 11 padding,
body text 15-20.

Title text: `DARKNESS OF KABUL`. Build label: `PROTOTYPE BETA  |  BUILD 0.2.0`.

## Steps for the first screen

1. Content Browser, right-click, **User Interface, Widget Blueprint**.
2. Pick **UKabulPrototypeUI** as the parent class, not UserWidget.
3. Name it `WBP_KabulUI`, save under `Content/UI`.
4. Add a Canvas Panel, then inside it a container named `MainMenuPanel`.
5. Add the four buttons using the names in the table.
6. Compile. Unreal reports any name it expected and did not find.
7. In `KabulPrototypeUI.h`, change `MainMenuPanel` and the four buttons to
   `UPROPERTY(meta = (BindWidget))`, delete `BuildMainMenuPanel()`, and bind the
   clicks in `NativeConstruct` instead.
8. Point the player's widget creation at `WBP_KabulUI`.
9. Build, play, confirm the main menu works.
10. Repeat per screen, deleting each `Build*Panel()` as its assets land.

## Finishing

When every screen is migrated, delete the `KabulUI` helper namespace and the
`BuildInterface` chain. Expect roughly 1,079 lines to fall to around 350.
