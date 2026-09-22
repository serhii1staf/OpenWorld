#include "SaveGame/OWSaveSubsystem.h"
#include "SaveGame/OWSaveData.h"
#include "SaveGame/OWWorldStateSubsystem.h"
#include "Character/OWCharacter.h"
#include "Character/OWHealthComponent.h"
#include "Weapons/OWWeaponComponent.h"
#include "Weapons/OWWeaponDefinition.h"
#include "Weapons/OWBallisticsSubsystem.h"
#include "Core/OWPlayerState.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
namespace { const FString SlotNames[] = { TEXT("OpenWorld_A"), TEXT("OpenWorld_B") }; }
bool UOWSaveSubsystem::Context(AOWCharacter*& C, AOWPlayerState*& P, AOWGameState*& S) const
{
    UWorld* W = GetWorld();
    if (!W || W->GetNetMode() != NM_Standalone) return false;
    APlayerController* PC = W->GetFirstPlayerController();
    C = PC ? Cast<AOWCharacter>(PC->GetPawn()) : nullptr;
    P = PC ? PC->GetPlayerState<AOWPlayerState>() : nullptr;
    S = W->GetGameState<AOWGameState>();
    return C && P && S && C->HasAuthority();
}
void UOWSaveSubsystem::Save()
{
    if (bBusy) return;
    AOWCharacter* C; AOWPlayerState* P; AOWGameState* S;
    if (!Context(C,P,S) || C->Health->IsDead()) { Finish(false, NSLOCTEXT("OpenWorld","SaveContext","Save requires a living player on foot in single-player.")); return; }
    Pending = Cast<UOWSaveData>(UGameplayStatics::CreateSaveGameObject(UOWSaveData::StaticClass()));
    Pending->Map = UGameplayStatics::GetCurrentLevelName(this, true);
    Pending->PlayerTransform = C->GetActorTransform(); Pending->Health = C->Health->GetHealth(); Pending->Magazine = C->Weapon->Magazine;
    Pending->Items = P->Inventory->GetItems(); Pending->Money = P->Inventory->GetMoney(); Pending->Missions = P->Missions->Progress;
    Pending->World = GetWorld()->GetSubsystem<UOWWorldStateSubsystem>()->Snapshot();
    Pending->WorldHours = S->WorldHours; Pending->Wetness = S->Wetness; Pending->Rain = S->Rain; Pending->Snow = S->Snow; Pending->Weather = S->Weather;
    if (!Validate(Pending)) { Finish(false, NSLOCTEXT("OpenWorld","SaveInvalid","Current state failed validation.")); return; }
    RequestCharacter = C; bSaving = true; bBusy = true; BeginRead();
}
void UOWSaveSubsystem::Load()
{
    if (bBusy) return;
    AOWCharacter* C; AOWPlayerState* P; AOWGameState* S;
    if (!Context(C,P,S)) { Finish(false, NSLOCTEXT("OpenWorld","LoadContext","Load requires a player on foot in single-player.")); return; }
    RequestCharacter = C; bSaving = false; bBusy = true; BeginRead();
}
void UOWSaveSubsystem::BeginRead()
{
    SlotA = nullptr; SlotB = nullptr; ReadsRemaining = 2;
    for (const FString& Slot : SlotNames)
        UGameplayStatics::AsyncLoadGameFromSlot(Slot, 0, FAsyncLoadGameFromSlotDelegate::CreateUObject(this, &UOWSaveSubsystem::ReadDone));
}
void UOWSaveSubsystem::ReadDone(const FString& Slot, int32, USaveGame* Object)
{
    UOWSaveData* Data = Cast<UOWSaveData>(Object);
    if (Slot == SlotNames[0]) SlotA = Data; else SlotB = Data;
    if (--ReadsRemaining != 0) return;
    AOWCharacter* CurrentCharacter; AOWPlayerState* CurrentPlayer; AOWGameState* CurrentState;
    if (!Context(CurrentCharacter, CurrentPlayer, CurrentState) || RequestCharacter.Get() != CurrentCharacter)
    {
        Finish(false, NSLOCTEXT("OpenWorld","SaveContextChanged","Player context changed during I/O; no save was applied or overwritten.")); return;
    }
    UOWSaveData* A = Validate(SlotA) ? SlotA.Get() : nullptr;
    UOWSaveData* B = Validate(SlotB) ? SlotB.Get() : nullptr;
    UOWSaveData* Latest = (!A || (B && B->Generation > A->Generation)) ? B : A;
    if (bSaving)
    {
        // Keep the last valid slot intact until the new asynchronous write succeeds.
        const FString Destination = Latest == SlotA.Get() && Latest ? SlotNames[1] : SlotNames[0];
        Pending->Generation = Latest ? Latest->Generation + 1 : 1;
        UGameplayStatics::AsyncSaveGameToSlot(Pending, Destination, 0, FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UOWSaveSubsystem::WriteDone));
    }
    else
    {
        if (!Latest) { Finish(false, NSLOCTEXT("OpenWorld","NoSave","No compatible valid save found.")); return; }
        const bool bApplied = Apply(Latest);
        Finish(bApplied, bApplied ? NSLOCTEXT("OpenWorld","Loaded","Loaded.") : NSLOCTEXT("OpenWorld","LoadBlocked","Saved position is not safely loaded. Move closer to that region and retry."));
    }
}
bool UOWSaveSubsystem::Validate(const UOWSaveData* D) const
{
    AOWCharacter* C; AOWPlayerState* P; AOWGameState* S;
    if (!D || !Context(C,P,S) || D->Schema != 1 || D->Generation < 0 || D->Generation >= MAX_int64 - 1 || D->Map != UGameplayStatics::GetCurrentLevelName(this,true)) return false;
    const int32 MaxMagazine = C->Weapon->Definition ? FMath::Clamp(C->Weapon->Definition->MagazineSize,1,200) : 0;
    if (D->PlayerTransform.ContainsNaN() || !D->PlayerTransform.GetRotation().IsNormalized() || !D->PlayerTransform.GetScale3D().Equals(FVector::OneVector,0.01f)) return false;
    if (!FMath::IsFinite(D->Health) || D->Health <= 0.f || D->Health > C->Health->MaxHealth || D->Magazine < 0 || D->Magazine > MaxMagazine) return false;
    if (!FMath::IsFinite(D->WorldHours) || D->WorldHours < 0 || D->WorldHours > 1000000 || !FMath::IsFinite(D->Wetness) || D->Wetness < 0 || D->Wetness > 1 || !FMath::IsFinite(D->Rain) || D->Rain < 0 || D->Rain > 1 || !FMath::IsFinite(D->Snow) || D->Snow < 0 || D->Snow > 1 || static_cast<uint8>(D->Weather) > static_cast<uint8>(EOWWeather::Snow)) return false;
    return UOWInventoryComponent::ValidateSnapshot(D->Items,D->Money) && P->Missions->ValidateSnapshot(D->Missions) && UOWWorldStateSubsystem::ValidateSnapshot(D->World);
}
bool UOWSaveSubsystem::Apply(UOWSaveData* D)
{
    AOWCharacter* C; AOWPlayerState* P; AOWGameState* S;
    if (!Context(C,P,S) || !Validate(D)) return false;
    // Until an async WP restore coordinator is implemented, never teleport into unloaded collision.
    const FVector Position = D->PlayerTransform.GetLocation();
    if (FVector::DistSquared(Position,C->GetActorLocation()) > FMath::Square(10000.0)) return false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(OWSaveRestore),false,C);
    FHitResult Floor;
    if (!GetWorld()->LineTraceSingleByChannel(Floor,Position,Position-FVector(0,0,500),ECC_Visibility,Params) || Floor.ImpactNormal.Z < 0.7f) return false;
    const FCollisionShape Capsule = FCollisionShape::MakeCapsule(C->GetCapsuleComponent()->GetScaledCapsuleRadius(), C->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    auto* WorldState = GetWorld()->GetSubsystem<UOWWorldStateSubsystem>();
    const TArray<FOWWorldRecord> Before = WorldState->Snapshot();
    if (!WorldState->Restore(D->World)) return false;
    if (GetWorld()->OverlapBlockingTestByChannel(Position,FQuat::Identity,ECC_Pawn,Capsule,Params))
    {
        WorldState->Restore(Before); return false;
    }
    C->Weapon->CancelActions(); GetWorld()->GetSubsystem<UOWBallisticsSubsystem>()->Clear();
    C->GetCharacterMovement()->StopMovementImmediately();
    C->SetActorLocationAndRotation(Position,D->PlayerTransform.Rotator(),false,nullptr,ETeleportType::TeleportPhysics);
    if (C->GetController()) C->GetController()->SetControlRotation(D->PlayerTransform.Rotator());
    C->Health->Restore(D->Health); C->Weapon->RestoreMagazine(D->Magazine);
    P->Inventory->Restore(D->Items,D->Money); P->Missions->Restore(D->Missions);
    S->WorldHours = D->WorldHours; S->Wetness = D->Wetness; S->Rain = D->Rain; S->Snow = D->Snow; S->Weather = D->Weather;
    return true;
}
void UOWSaveSubsystem::WriteDone(const FString&, int32, bool bSuccess)
{
    Finish(bSuccess, bSuccess ? NSLOCTEXT("OpenWorld","Saved","Saved.") : NSLOCTEXT("OpenWorld","SaveFailed","Save failed. The previous valid slot was retained."));
}
void UOWSaveSubsystem::Finish(bool bSuccess, const FText& Message)
{
    RequestCharacter.Reset(); bBusy = false; Pending = nullptr; SlotA = nullptr; SlotB = nullptr;
    OnResult.Broadcast(bSuccess, Message);
}
