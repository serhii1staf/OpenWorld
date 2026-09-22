#include "Missions/OWMissionComponent.h"
#include "Inventory/OWInventoryComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
UOWMissionComponent::UOWMissionComponent()
{
    SetIsReplicatedByDefault(true); PrimaryComponentTick.bCanEverTick = false;
}
const UOWMissionDefinition* UOWMissionComponent::Find(FName Id) const
{
    for (const auto& D : Definitions) if (D && D->MissionId == Id) return D;
    return nullptr;
}
bool UOWMissionComponent::Start(FName Id)
{
    if (!GetOwner()->HasAuthority() || Progress.Num() >= 128) return false;
    const UOWMissionDefinition* D = Find(Id);
    if (!D || D->Objectives.IsEmpty() || D->Reward < 0 || Progress.ContainsByPredicate([Id](const FOWMissionProgress& P){return P.MissionId == Id;})) return false;
    for (const FOWObjective& O : D->Objectives) if (O.Event.IsNone() || O.Required < 1) return false;
    for (FName Required : D->Prerequisites)
        if (!Progress.ContainsByPredicate([Required](const FOWMissionProgress& P){return P.MissionId == Required && P.bComplete;})) return false;
    for (const FOWMissionProgress& P : Progress)
    {
        const UOWMissionDefinition* Other = Find(P.MissionId);
        if (D->Excludes.Contains(P.MissionId) || (Other && Other->Excludes.Contains(Id))) return false;
    }
    FOWMissionProgress P; P.MissionId = Id; Progress.Add(P); Changed(); return true;
}
void UOWMissionComponent::Emit(FName Event, int32 Amount)
{
    if (!GetOwner()->HasAuthority() || Event.IsNone() || Amount < 1 || Amount > 10000) return;
    bool bChanged = false;
    for (FOWMissionProgress& P : Progress)
    {
        const UOWMissionDefinition* D = Find(P.MissionId);
        if (P.bComplete || !D || !D->Objectives.IsValidIndex(P.Objective)) continue;
        const FOWObjective& O = D->Objectives[P.Objective];
        if (O.Event != Event) continue;
        const int32 NewCount = static_cast<int32>(FMath::Min<int64>(O.Required, static_cast<int64>(P.Count) + Amount));
        if (NewCount == O.Required && P.Objective + 1 == D->Objectives.Num())
        {
            auto* Inventory = GetOwner()->FindComponentByClass<UOWInventoryComponent>();
            // Reward and completion form a single authority-side operation: no duplicate rewards on replay.
            if (!Inventory || !Inventory->Credit(D->Reward)) continue;
            P.bComplete = true;
        }
        P.Count = NewCount; bChanged = true;
        if (P.Count == O.Required) { ++P.Objective; P.Count = 0; }
    }
    if (bChanged) Changed();
}
bool UOWMissionComponent::ValidateSnapshot(const TArray<FOWMissionProgress>& Data) const
{
    if (Data.Num() > 128) return false;
    TSet<FName> Seen;
    for (const FOWMissionProgress& P : Data)
    {
        const UOWMissionDefinition* D = Find(P.MissionId);
        if (!D || D->Objectives.IsEmpty() || Seen.Contains(P.MissionId)) return false;
        Seen.Add(P.MissionId);
        if (P.bComplete)
        {
            if (P.Objective != D->Objectives.Num() || P.Count != 0) return false;
        }
        else if (!D->Objectives.IsValidIndex(P.Objective) || P.Count < 0 || P.Count >= D->Objectives[P.Objective].Required) return false;
    }
    return true;
}
bool UOWMissionComponent::Restore(const TArray<FOWMissionProgress>& Data)
{
    if (!GetOwner()->HasAuthority() || !ValidateSnapshot(Data)) return false;
    Progress = Data; Changed(); return true;
}
void UOWMissionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const
{
    Super::GetLifetimeReplicatedProps(Out);
    DOREPLIFETIME_CONDITION(UOWMissionComponent, Progress, COND_OwnerOnly);
}
