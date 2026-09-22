#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SaveGame/OWPersistentEntity.h"
#include "OWWorldStateSubsystem.generated.h"
UCLASS()
class OPENWORLD_API UOWWorldStateSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    bool Register(AActor* Actor);
    void Unregister(AActor* Actor);
    TArray<FOWWorldRecord> Snapshot();
    bool Restore(const TArray<FOWWorldRecord>& Data);
    static bool ValidateSnapshot(const TArray<FOWWorldRecord>& Data);
private:
    TMap<FGuid, TWeakObjectPtr<AActor>> Active;
    TMap<FGuid, FOWWorldRecord> Records;
    TMap<FGuid, FOWWorldRecord> Defaults;
};
