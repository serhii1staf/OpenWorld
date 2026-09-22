#include "Environment/OWEnvironmentDirector.h"
#include "Environment/OWGameState.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
AOWEnvironmentDirector::AOWEnvironmentDirector() { PrimaryActorTick.bCanEverTick = false; }
void AOWEnvironmentDirector::BeginPlay()
{
    Super::BeginPlay();
    if (GetNetMode() != NM_DedicatedServer)
    {
        GetWorldTimerManager().SetTimer(UpdateTimer, this, &AOWEnvironmentDirector::UpdateEnvironment, 0.2f, true);
        UpdateEnvironment();
    }
}
void AOWEnvironmentDirector::UpdateEnvironment()
{
    const AOWGameState* S = GetWorld()->GetGameState<AOWGameState>(); if (!S) return;
    const float Hour = S->HourOfDay();
    const float SunAngle = (Hour - 6.f) * 15.f;
    const float Day = FMath::Clamp(FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 5.f, 0.f, 1.f);
    if (Sun)
    {
        Sun->SetActorRotation(FRotator(-SunAngle, 20.f, 0));
        Sun->GetLightComponent()->SetIntensity(DayLux * Day * (1.f - S->Rain * 0.75f));
    }
    if (Moon)
    {
        Moon->SetActorRotation(FRotator(-SunAngle + 180.f, 20.f, 0));
        Moon->GetLightComponent()->SetIntensity(MoonLux * (1.f - Day));
    }
    if (Fog) Fog->GetComponent()->SetFogDensity(S->Weather == EOWWeather::Fog ? 0.06f : FMath::Lerp(0.005f, 0.025f, S->Rain));
    if (WeatherParameters)
    {
        UKismetMaterialLibrary::SetScalarParameterValue(this, WeatherParameters, "Rain", S->Rain);
        UKismetMaterialLibrary::SetScalarParameterValue(this, WeatherParameters, "Wetness", S->Wetness);
        UKismetMaterialLibrary::SetScalarParameterValue(this, WeatherParameters, "Snow", S->Snow);
        UKismetMaterialLibrary::SetScalarParameterValue(this, WeatherParameters, "Night", 1.f - Day);
    }
    EnvironmentUpdated(Hour, S->Rain, S->Wetness, S->Snow);
}
void AOWEnvironmentDirector::EndPlay(const EEndPlayReason::Type Reason) { GetWorldTimerManager().ClearTimer(UpdateTimer); Super::EndPlay(Reason); }
