#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Missions/OWMissionDefinition.h"
#include "OWMissionComponent.generated.h"
USTRUCT(BlueprintType)
struct FOWMissionProgress
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, SaveGame) FName MissionId;
    UPROPERTY(BlueprintReadOnly, SaveGame) int32 Objective = 0;
    UPROPERTY(BlueprintReadOnly, SaveGame) int32 Count = 0;
    UPROPERTY(BlueprintReadOnly, SaveGame) bool bComplete = false;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOWMissionsChanged);
UCLASS(ClassGroup=(OpenWorld), meta=(BlueprintSpawnableComponent))
class OPENWORLD_API UOWMissionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UOWMissionComponent();
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TArray<TObjectPtr<UOWMissionDefinition>> Definitions;
    UPROPERTY(ReplicatedUsing=Changed, BlueprintReadOnly, SaveGame) TArray<FOWMissionProgress> Progress;
    UPROPERTY(BlueprintAssignable) FOWMissionsChanged OnChanged;
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly) bool Start(FName Id);
    // Only authoritative gameplay systems may emit events. No client event RPC exists.
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly) void Emit(FName Event, int32 Amount = 1);
    bool ValidateSnapshot(const TArray<FOWMissionProgress>& Data) const;
    bool Restore(const TArray<FOWMissionProgress>& Data);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
private:
    const UOWMissionDefinition* Find(FName Id) const;
    UFUNCTION() void Changed() { OnChanged.Broadcast(); }
};
