#include "KabulGameMode.h"

#include "KabulPlayerController.h"
#include "PlayerCharacter.h"

AKabulGameMode::AKabulGameMode()
{
	// Milestone one always starts with the movement character. A Blueprint child
	// may replace visuals later without replacing the locomotion code.
	DefaultPawnClass = APlayerCharacter::StaticClass();
	PlayerControllerClass = AKabulPlayerController::StaticClass();

	// Future game-wide rules belong here when their milestone begins:
	// objectives, win/loss, encounter state, and restart flow.
}
