#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OWTrafficLane.generated.h"
class USplineComponent;
UCLASS()
class OPENWORLD_API AOWTrafficSignal : public AActor
{
    GENERATED_BODY()
public:
    AOWTrafficSignal();
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float GreenSeconds = 20.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float ClearanceSeconds = 3.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float PhaseOffset = 0.f;
    UFUNCTION(BlueprintPure) bool IsGreen(bool bGroupB) const;
};
UCLASS()
class OPENWORLD_API AOWTrafficLane : public AActor
{
    GENERATED_BODY()
public:
    AOWTrafficLane();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USplineComponent> Spline;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float SpeedLimitKPH = 40.f;
    // Soft cross-cell references: never synchronously load a remote road actor.
    UPROPERTY(EditInstanceOnly) TSoftObjectPtr<AOWTrafficLane> Next;
    UPROPERTY(EditInstanceOnly) TSoftObjectPtr<AOWTrafficSignal> Signal;
    UPROPERTY(EditInstanceOnly) bool bSignalGroupB = false;
    UPROPERTY(EditInstanceOnly) float StopLineDistance = -1.f;
};
