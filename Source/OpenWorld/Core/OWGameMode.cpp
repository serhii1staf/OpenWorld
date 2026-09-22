#include "Core/OWGameMode.h"
#include "Core/OWPlayerState.h"
#include "Core/OWPlayerController.h"
#include "Character/OWCharacter.h"
#include "Environment/OWGameState.h"
#include "UI/OWHUD.h"
AOWGameMode::AOWGameMode()
{
    DefaultPawnClass = AOWCharacter::StaticClass();
    PlayerStateClass = AOWPlayerState::StaticClass();
    PlayerControllerClass = AOWPlayerController::StaticClass();
    GameStateClass = AOWGameState::StaticClass(); HUDClass = AOWHUD::StaticClass();
}
