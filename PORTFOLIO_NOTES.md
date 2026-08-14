# Portfolio Notes

## Description

Darkness of Kabul is an Unreal C++ gameplay prototype built around a repeatable slingshot interaction. It demonstrates compact two-impact ricochet prediction, swept projectile bounce, enemy reactions selected from the struck body area, and a complete native UMG prototype flow.

## Role

Gameplay programming and prototype integration: first-person input, slingshot state, trajectory prediction, projectile movement, collision, damage routing, physical reaction states, objectives, UMG menus/settings/pause/completion, reset flow, and Windows build validation.

## Verified technical facts

- One shared charge calculation drives the ready-state preview and real launch speed from 6,000 to 9,200 cm/s.
- The real stone and predictor share collision radius, gravity, bounciness, friction, the `StoneProjectile` channel, an eight-second lifetime, and a maximum of two contacts.
- Instead of a full path line, UMG renders a body marker for zombie hits and only short angle pieces plus the final marker for surface ricochets.
- The original 3.5-second overdraw failure is preserved: it cancels aim and drives the camera kick before the player can aim again.
- Projectile movement uses swept collision, forced sub-stepping, and a stop-speed threshold.
- Physics Asset bone names resolve to head, torso, arm, or leg; an unavailable name uses the closest physical bone and then a torso fallback.
- Torso hits stagger, leg hits enter crawling, head or lethal hits activate ragdoll, and `R` restores the player start, placed enemies, objective progress, and stones.
- Native UMG supplies loading, welcome, main menu, disabled Continue, settings, gameplay HUD, pause, and completion screens.

## Prototype boundary

This is a physics and reaction demonstration. It is not a finished game, a full combat system, a save system, or a claim of procedural animation.

## Recording sequence (20-30 seconds)

1. Show the loading, welcome, main-menu, and settings screens briefly; choose New Game.
2. Hold right mouse to aim and left mouse for about one second so the marker changes from orange to green.
3. Frame a wall angle: show the short angle piece and the final marker on a zombie, then release for an indirect hit.
4. Hold the reaction for two seconds so the stagger or crawl state reads clearly.
5. Press `R`, show `SCENARIO RESET`, and finish by opening the pause menu.

## Recording settings

- Use the Win64 Shipping package at 1920x1080 fullscreen or windowed.
- Record at 60 fps when stable; use 30 fps if capture affects frame pacing.
- Keep the packaged UMG visible and exclude editor windows and development overlays.
- Disable the machine's external FPS/GPU/CPU/LAT overlay before recording.
- Use the existing target-practice lighting and avoid exposure changes during the take.

## Disclosed limitations

The short UMG loading screen is a post-map-load presentation rather than an asynchronous movie-player screen. The exact one-wall camera angle requires a short hands-on rehearsal in the packaged map. External asset licence records must be completed before the build is published publicly.
