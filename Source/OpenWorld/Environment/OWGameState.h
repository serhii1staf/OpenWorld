#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "OWGameState.generated.h"
UENUM(BlueprintType)
enum class EOWWeather : uint8 { Clear, Cloudy, Rain, Storm, Fog, Snow };
UCLASS()
class OPENWORLD_API AOWGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    AOWGameState();
    UPROPERTY(Replicated, BlueprintReadOnly, SaveGame) double WorldHours = 8.0;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float GameHoursPerRealMinute = 0.5f;
    UPROPERTY(Replicated, BlueprintReadOnly, SaveGame) EOWWeather Weather = EOWWeather::Clear;
    UPROPERTY(Replicated, BlueprintReadOnly, SaveGame) float Wetness = 0.f;
    UPROPERTY(Replicated, BlueprintReadOnly) float Rain = 0.f;
    UPROPERTY(Replicated, BlueprintReadOnly) float Snow = 0.f;
    UFUNCTION(BlueprintPure) float HourOfDay() const { return static_cast<float>(FMath::Fmod(WorldHours, 24.0)); }
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    FTimerHandle ClockTimer;
    void Advance();
};
