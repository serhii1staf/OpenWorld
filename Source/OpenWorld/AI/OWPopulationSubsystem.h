#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OWPopulationSubsystem.generated.h"
class AOWCitizen;
UCLASS()
class OPENWORLD_API UOWPopulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void OnWorldBeginPlay(UWorld& World) override;
    virtual void Deinitialize() override;
    void Register(AOWCitizen* Citizen);
    void Unregister(AOWCitizen* Citizen);
private:
    TArray<TWeakObjectPtr<AOWCitizen>> Citizens;
    int32 Cursor = 0;
    FTimerHandle Timer;
    void Service();
};
