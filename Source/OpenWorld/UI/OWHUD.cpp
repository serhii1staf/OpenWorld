#include "UI/OWHUD.h"
#include "Core/OWPlayerController.h"
#include "Core/OWPlayerState.h"
#include "Character/OWCharacter.h"
#include "Character/OWHealthComponent.h"
#include "Weapons/OWWeaponComponent.h"
#include "Inventory/OWInventoryComponent.h"
#include "Missions/OWMissionComponent.h"
#include "Interaction/OWInteractable.h"
#include "Engine/Canvas.h"
void AOWHUD::DrawHUD()
{
    Super::DrawHUD();
    AOWPlayerController* PC = Cast<AOWPlayerController>(GetOwningPlayerController());
    if (!PC || !Canvas || PC->IsPaused()) return;
    const float X = 40.f; const float Y = Canvas->ClipY - 90.f;
    const FLinearColor Ink(0.88f,0.93f,0.94f);
    if (AOWCharacter* C = Cast<AOWCharacter>(PC->GetPawn()))
    {
        DrawRect(FLinearColor(0.04f,0.055f,0.06f,0.8f),X,Y,180.f,4.f);
        DrawRect(FLinearColor(0.28f,0.78f,0.61f),X,Y,180.f * C->Health->GetHealth()/FMath::Max(1.f,C->Health->MaxHealth),4.f);
        DrawText(FString::Printf(TEXT("%d  /  %s"),C->Weapon->Magazine,C->Weapon->bReloading ? TEXT("RELOADING") : TEXT("AMMO")),Ink,X,Y+15.f);
        DrawRect(Ink,Canvas->ClipX*0.5f-1.f,Canvas->ClipY*0.5f-1.f,2.f,2.f);
        if (AActor* A = C->FindInteraction())
        {
            if (IOWInteractable::Execute_CanInteract(A,C)) DrawText(TEXT("E  ")+IOWInteractable::Execute_GetPrompt(A,C).ToString(),Ink,Canvas->ClipX*0.5f-60.f,Canvas->ClipY*0.6f);
        }
    }
    if (const AOWPlayerState* P = PC->GetPlayerState<AOWPlayerState>())
    {
        DrawText(FString::Printf(TEXT("$ %lld"),static_cast<long long>(P->Inventory->GetMoney())),Ink,X,40.f);
        float Row = 80.f;
        for (const FOWMissionProgress& M : P->Missions->Progress)
        {
            if (M.bComplete) continue;
            for (const auto& D : P->Missions->Definitions)
                if (D && D->MissionId == M.MissionId && D->Objectives.IsValidIndex(M.Objective))
                {
                    const FOWObjective& O = D->Objectives[M.Objective];
                    DrawText(D->Title.ToString()+TEXT(" / ")+O.Description.ToString()+FString::Printf(TEXT(" (%d/%d)"),M.Count,O.Required),Ink,X,Row); Row += 24.f;
                }
        }
    }
    DrawText(PC->LastStatus.ToString(),Ink,X,Canvas->ClipY-30.f);
}
