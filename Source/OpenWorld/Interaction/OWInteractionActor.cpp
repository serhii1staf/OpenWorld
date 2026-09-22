#include "Interaction/OWInteractionActor.h"
#include "Components/StaticMeshComponent.h"
#include "Core/OWPlayerState.h"
#include "Inventory/OWInventoryComponent.h"
#include "Missions/OWMissionComponent.h"
#include "SaveGame/OWWorldStateSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
AOWInteractionActor::AOWInteractionActor()
{
    bReplicates = true; PrimaryActorTick.bCanEverTick = false;
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh")); SetRootComponent(Mesh);
    Prompt = NSLOCTEXT("OpenWorld", "Interact", "Interact");
}
void AOWInteractionActor::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority() && !GetWorld()->GetSubsystem<UOWWorldStateSubsystem>()->Register(this))
        UE_LOG(LogTemp, Error, TEXT("Interaction actor needs a unique saved GUID: %s"), *GetPathName());
}
void AOWInteractionActor::EndPlay(const EEndPlayReason::Type Reason)
{
    if (HasAuthority()) GetWorld()->GetSubsystem<UOWWorldStateSubsystem>()->Unregister(this);
    Super::EndPlay(Reason);
}
FText AOWInteractionActor::GetPrompt_Implementation(APawn*) const { return bUsed ? FText::GetEmpty() : Prompt; }
bool AOWInteractionActor::CanInteract_Implementation(APawn* User) const
{
    const AOWPlayerState* P = User ? User->GetPlayerState<AOWPlayerState>() : nullptr;
    return P && !bUsed && FVector::DistSquared(User->GetActorLocation(), GetActorLocation()) <= FMath::Square(400.f)
        && Cost >= 0 && P->Inventory->GetMoney() >= Cost && (RequiredItem.IsNone() || P->Inventory->Count(RequiredItem) > 0)
        && (GrantedItem.IsNone() || P->Inventory->Count(GrantedItem) < 9999);
}
void AOWInteractionActor::Interact_Implementation(APawn* User)
{
    if (!HasAuthority() || !CanInteract_Implementation(User)) return;
    AOWPlayerState* P = User->GetPlayerState<AOWPlayerState>();
    TArray<FOWItemStack> Items = P->Inventory->GetItems();
    if (!RequiredItem.IsNone())
    {
        const int32 Index = Items.IndexOfByPredicate([this](const FOWItemStack& S){ return S.Item == RequiredItem; });
        if (Index == INDEX_NONE) return;
        if (--Items[Index].Count == 0) Items.RemoveAtSwap(Index);
    }
    if (!GrantedItem.IsNone())
    {
        FOWItemStack* Stack = Items.FindByPredicate([this](const FOWItemStack& S){ return S.Item == GrantedItem; });
        if (Stack) ++Stack->Count;
        else { FOWItemStack S; S.Item = GrantedItem; S.Count = 1; Items.Add(S); }
    }
    const int64 NewMoney = P->Inventory->GetMoney() - Cost;
    if (!UOWInventoryComponent::ValidateSnapshot(Items, NewMoney)) return;
    bUsed = true;
    if (!MissionToStart.IsNone() && !P->Missions->Start(MissionToStart)) { bUsed = false; return; }
    P->Inventory->Restore(Items, NewMoney);
    bUsed = true; StateChanged();
    if (!MissionEvent.IsNone()) P->Missions->Emit(MissionEvent);
}
void AOWInteractionActor::StateChanged() { PresentationChanged(bUsed); }
FOWWorldRecord AOWInteractionActor::CapturePersistentState() const
{
    FOWWorldRecord R; R.Id = SaveId; R.Kind = "Interaction"; R.Transform = GetActorTransform(); R.bUsed = bUsed; return R;
}
void AOWInteractionActor::RestorePersistentState(const FOWWorldRecord& R)
{
    if (HasAuthority() && R.Id == SaveId && R.Kind == "Interaction") { bUsed = R.bUsed; StateChanged(); }
}
void AOWInteractionActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(AOWInteractionActor, bUsed);
}
