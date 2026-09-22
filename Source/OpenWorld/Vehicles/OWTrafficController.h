#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "OWTrafficController.generated.h"
class AOWTrafficLane;
UCLASS()
class OPENWORLD_API AOWTrafficController : public AAIController
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly) void SetLane(AOWTrafficLane* NewLane);
protected:
    virtual void OnPossess(APawn* Pawn) override;
    virtual void OnUnPossess() override;
private:
    TWeakObjectPtr<AOWTrafficLane> Lane;
    FTimerHandle Timer;
    void Drive();
};
