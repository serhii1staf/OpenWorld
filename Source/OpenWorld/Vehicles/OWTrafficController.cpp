#include "Vehicles/OWTrafficController.h"
#include "Vehicles/OWTrafficLane.h"
#include "Vehicles/OWVehiclePawn.h"
#include "Character/OWHealthComponent.h"
#include "Components/SplineComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
void AOWTrafficController::SetLane(AOWTrafficLane* NewLane) { if (HasAuthority()) Lane = NewLane; }
void AOWTrafficController::OnPossess(APawn* P)
{
    Super::OnPossess(P);
    if (HasAuthority()) GetWorldTimerManager().SetTimer(Timer,this,&AOWTrafficController::Drive,0.1f,true);
}
void AOWTrafficController::OnUnPossess()
{
    GetWorldTimerManager().ClearTimer(Timer); Super::OnUnPossess();
}
void AOWTrafficController::Drive()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(OW_Traffic);
    AOWVehiclePawn* V = Cast<AOWVehiclePawn>(GetPawn()); if (!V) return;
    UChaosVehicleMovementComponent* Movement = V->GetVehicleMovementComponent();
    AOWTrafficLane* L = Lane.Get();
    if (!L || V->Health->IsDead()) { Movement->SetThrottleInput(0); Movement->SetBrakeInput(1); return; }
    const FVector Position = V->GetActorLocation();
    const float Distance = L->Spline->GetDistanceAlongSplineAtLocation(Position,ESplineCoordinateSpace::World);
    const float Speed = FMath::Max(0.f,Movement->GetForwardSpeed());
    const float LookAhead = FMath::Clamp(Speed*0.8f,400.f,1800.f);
    const FVector Aim = L->Spline->GetLocationAtDistanceAlongSpline(Distance+LookAhead,ESplineCoordinateSpace::World);
    const FVector Local = V->GetActorTransform().InverseTransformPosition(Aim);
    const float Steering = FMath::Clamp(FMath::Atan2(Local.Y,Local.X)*1.5f,-1.f,1.f);
    float Desired = FMath::Clamp(L->SpeedLimitKPH,5.f,90.f)*100.f/3.6f*(1.f-0.65f*FMath::Abs(Steering));
    const float Remaining = L->Spline->GetSplineLength()-Distance;
    if (!L->Next.IsValid()) Desired = FMath::Min(Desired,FMath::Sqrt(2.f*300.f*FMath::Max(0.f,Remaining-250.f)));
    if (!L->Signal.IsNull() && L->StopLineDistance >= 0)
    {
        AOWTrafficSignal* Signal = L->Signal.Get();
        const float ToStop = L->StopLineDistance-Distance;
        if ((!Signal || !Signal->IsGreen(L->bSignalGroupB)) && ToStop >= -50.f)
            Desired = FMath::Min(Desired,FMath::Sqrt(2.f*400.f*FMath::Max(0.f,ToStop-150.f)));
    }
    FHitResult Hit; FCollisionQueryParams Params(SCENE_QUERY_STAT(OWTrafficObstacle),false,V);
    const FVector Start = Position+FVector(0,0,90.f)+V->GetActorForwardVector()*180.f;
    const float Probe = FMath::Clamp(300.f+Speed*1.5f,400.f,4000.f);
    if (GetWorld()->SweepSingleByChannel(Hit,Start,Start+V->GetActorForwardVector()*Probe,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(60.f),Params))
        Desired = FMath::Min(Desired,FMath::Sqrt(2.f*500.f*FMath::Max(0.f,Hit.Distance-300.f)));
    Movement->SetSteeringInput(Steering); Movement->SetTargetGear(1,true);
    Movement->SetThrottleInput(FMath::Clamp((Desired-Speed)/500.f,0.f,0.7f));
    Movement->SetBrakeInput(FMath::Clamp((Speed-Desired)/300.f,0.f,1.f));
    if (Remaining < 150.f && L->Next.IsValid()) Lane = L->Next.Get();
}
