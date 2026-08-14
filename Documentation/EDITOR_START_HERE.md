# Editor and Playtest Guide

1. Open `DarknessOfKabul.uproject` in Unreal Engine 5.6.1.
2. Open `/Game/Maps/L_TargetPractice` if it is not already active.
3. Press Play and click the game window.
4. Continue through the welcome screen and choose New Game.

## Core playtest

1. Hold right mouse to aim.
2. Hold left mouse until the charge changes from orange to green.
3. Aim directly at a zombie and confirm the marker sits on the body without a full trajectory line.
4. Aim at a wall and confirm the preview shows a short angle at the first impact and a final marker after the bounce.
5. Release and confirm the stone stops after the second collision at the latest.
6. Hold a pull for 3.5 seconds and confirm the pull cancels, aim resets, and the camera kicks.
7. Move away from the start, press `R`, and confirm the player, targets, stones, and objective progress reset.

## UI playtest

- Main menu: New Game works, Continue is disabled, Settings opens, and Leave exits.
- Settings: volume, look sensitivity, field of view, and display mode update correctly.
- Pause: Escape opens the menu; Resume, Play Again, Settings, Main Menu, and Leave work.
- Completion: defeating every zombie and hitting every wobble target opens the thank-you screen.

## Level editing

Placed zombies are discovered automatically. Actors whose class name contains `WobbleTarget` are discovered as wobble objectives. Adding or removing either type updates the required totals the next time the level starts.
