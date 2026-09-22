#include "SaveGame/OWWorldStateSubsystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
bool UOWWorldStateSubsystem::Register(AActor* Actor)
{
    IOWPersistentEntity* P = Cast<IOWPersistentEntity>(Actor);
    if (!P || !Actor->HasAuthority() || !P->PersistentId().IsValid()) return false;
    const FGuid Id = P->PersistentId();
    if (const TWeakObjectPtr<AActor>* Existing = Active.Find(Id))
    {
        if (Existing->IsValid() && Existing->Get() != Actor)
        {
            UE_LOG(LogTemp, Error, TEXT("Duplicate persistent GUID on %s"), *Actor->GetPathName()); return false;
        }
    }
    Active.Add(Id, Actor); Defaults.Add(Id, P->CapturePersistentState());
    if (const FOWWorldRecord* Record = Records.Find(Id)) P->RestorePersistentState(*Record);
    return true;
}
void UOWWorldStateSubsystem::Unregister(AActor* Actor)
{
    IOWPersistentEntity* P = Cast<IOWPersistentEntity>(Actor); if (!P) return;
    const FGuid Id = P->PersistentId();
    if (Active.FindRef(Id).Get() != Actor) return;
    Records.Add(Id, P->CapturePersistentState()); Active.Remove(Id); Defaults.Remove(Id);
}
TArray<FOWWorldRecord> UOWWorldStateSubsystem::Snapshot()
{
    for (auto It = Active.CreateIterator(); It; ++It)
    {
        if (IOWPersistentEntity* P = Cast<IOWPersistentEntity>(It.Value().Get())) Records.Add(It.Key(), P->CapturePersistentState());
        else It.RemoveCurrent();
    }
    TArray<FOWWorldRecord> Result; Records.GenerateValueArray(Result); return Result;
}
bool UOWWorldStateSubsystem::ValidateSnapshot(const TArray<FOWWorldRecord>& Data)
{
    if (Data.Num() > 50000) return false;
    TSet<FGuid> Seen;
    for (const FOWWorldRecord& R : Data)
    {
        if (!R.Id.IsValid() || Seen.Contains(R.Id) || R.Kind.IsNone() || R.Transform.ContainsNaN() || !R.Transform.GetRotation().IsNormalized() || !FMath::IsFinite(R.Health) || R.Health < 0.f || R.Health > 100.f) return false;
        Seen.Add(R.Id);
    }
    return true;
}
bool UOWWorldStateSubsystem::Restore(const TArray<FOWWorldRecord>& Data)
{
    if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client || !ValidateSnapshot(Data)) return false;
    Records.Reset(); for (const FOWWorldRecord& R : Data) Records.Add(R.Id, R);
    for (const auto& Pair : Active)
    {
        if (IOWPersistentEntity* P = Cast<IOWPersistentEntity>(Pair.Value.Get()))
        {
            const FOWWorldRecord* R = Records.Find(Pair.Key);
            if (!R) R = Defaults.Find(Pair.Key);
            if (R) P->RestorePersistentState(*R);
        }
    }
    return true;
}
