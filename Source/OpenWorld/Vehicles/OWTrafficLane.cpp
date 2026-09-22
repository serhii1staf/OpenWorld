#include "Vehicles/OWTrafficLane.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
AOWTrafficSignal::AOWTrafficSignal() { PrimaryActorTick.bCanEverTick = false; }
bool AOWTrafficSignal::IsGreen(bool bGroupB) const
{
    const AGameStateBase* State = GetWorld()->GetGameState();
    if (!State) return false;
    const double Green = FMath::Max(1.f,GreenSeconds), Clearance = FMath::Max(1.f,ClearanceSeconds);
    const double Cycle = 2.0*(Green+Clearance);
    const double T = FMath::Fmod(State->GetServerWorldTimeSeconds()+PhaseOffset+Cycle*1000.0,Cycle);
    return bGroupB ? T >= Green+Clearance && T < Green*2.0+Clearance : T >= 0 && T < Green;
}
AOWTrafficLane::AOWTrafficLane()
{
    PrimaryActorTick.bCanEverTick = false;
    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Lane")); SetRootComponent(Spline);
}
