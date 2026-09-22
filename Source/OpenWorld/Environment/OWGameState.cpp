#include "Environment/OWGameState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
AOWGameState::AOWGameState() { NetUpdateFrequency = 2.f; PrimaryActorTick.bCanEverTick = false; }
void AOWGameState::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority()) GetWorldTimerManager().SetTimer(ClockTimer, this, &AOWGameState::Advance, 0.5f, true);
}
void AOWGameState::Advance()
{
    WorldHours += FMath::Clamp(GameHoursPerRealMinute, 0.f, 10.f) / 120.0;
    const int32 Phase = static_cast<int32>(FMath::FloorToDouble(WorldHours / 3.0)) % 12;
    static const EOWWeather Schedule[] = { EOWWeather::Clear, EOWWeather::Clear, EOWWeather::Cloudy, EOWWeather::Rain, EOWWeather::Storm, EOWWeather::Cloudy, EOWWeather::Fog, EOWWeather::Clear, EOWWeather::Cloudy, EOWWeather::Snow, EOWWeather::Cloudy, EOWWeather::Clear };
    Weather = Schedule[Phase];
    const float TargetRain = Weather == EOWWeather::Storm ? 1.f : Weather == EOWWeather::Rain ? 0.5f : 0.f;
    Rain = FMath::FInterpConstantTo(Rain, TargetRain, 0.5f, 0.03f);
    Snow = FMath::FInterpConstantTo(Snow, Weather == EOWWeather::Snow ? 1.f : 0.f, 0.5f, 0.02f);
    Wetness = FMath::FInterpConstantTo(Wetness, Rain > 0.1f ? 1.f : 0.f, 0.5f, Rain > 0.1f ? 0.02f : 0.002f);
}
void AOWGameState::EndPlay(const EEndPlayReason::Type Reason) { GetWorldTimerManager().ClearTimer(ClockTimer); Super::EndPlay(Reason); }
void AOWGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOWGameState, WorldHours); DOREPLIFETIME(AOWGameState, Weather);
    DOREPLIFETIME(AOWGameState, Wetness); DOREPLIFETIME(AOWGameState, Rain); DOREPLIFETIME(AOWGameState, Snow);
}
