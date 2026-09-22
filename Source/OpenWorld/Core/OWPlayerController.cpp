#include "Core/OWPlayerController.h"
#include "SaveGame/OWSaveSubsystem.h"
#include "Character/OWCharacter.h"
#include "Weapons/OWWeaponComponent.h"
#include "UI/OWPauseMenu.h"
#include "Components/InputComponent.h"
#include "Engine/GameInstance.h"
void AOWPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (IsLocalController()) GetGameInstance()->GetSubsystem<UOWSaveSubsystem>()->OnResult.AddDynamic(this,&AOWPlayerController::SaveResult);
}
void AOWPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    FInputActionBinding& PauseBinding = InputComponent->BindAction("Pause",IE_Pressed,this,&AOWPlayerController::TogglePause);
    PauseBinding.bExecuteWhenPaused = true;
    InputComponent->BindAction("Save",IE_Pressed,this,&AOWPlayerController::Save);
    InputComponent->BindAction("Load",IE_Pressed,this,&AOWPlayerController::Load);
}
void AOWPlayerController::TogglePause()
{
    if (!IsLocalController() || GetNetMode() != NM_Standalone) return;
    if (IsPaused())
    {
        if (PauseMenu) PauseMenu->RemoveFromParent();
        SetPause(false); bShowMouseCursor = false; SetInputMode(FInputModeGameOnly());
        SetIgnoreMoveInput(false); SetIgnoreLookInput(false);
    }
    else
    {
        if (AOWCharacter* C = Cast<AOWCharacter>(GetPawn())) C->Weapon->StopFire();
        if (!PauseMenu) PauseMenu = CreateWidget<UOWPauseMenu>(this,UOWPauseMenu::StaticClass());
        PauseMenu->AddToViewport(100); SetPause(true); bShowMouseCursor = true;
        SetIgnoreMoveInput(true); SetIgnoreLookInput(true);
        FInputModeGameAndUI Mode; Mode.SetWidgetToFocus(PauseMenu->TakeWidget()); SetInputMode(Mode);
    }
}
void AOWPlayerController::Save() { GetGameInstance()->GetSubsystem<UOWSaveSubsystem>()->Save(); }
void AOWPlayerController::Load() { GetGameInstance()->GetSubsystem<UOWSaveSubsystem>()->Load(); }
void AOWPlayerController::SaveResult(bool, FText Message) { LastStatus = Message; }
