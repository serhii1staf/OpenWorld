#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OWEnvironmentDirector.generated.h"
class ADirectionalLight;
class AExponentialHeightFog;
class UMaterialParameterCollection;
UCLASS()
class OPENWORLD_API AOWEnvironmentDirector : public AActor
{
    GENERATED_BODY()
public:
    AOWEnvironmentDirector();
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly) TObjectPtr<ADirectionalLight> Sun;
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly) TObjectPtr<ADirectionalLight> Moon;
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly) TObjectPtr<AExponentialHeightFog> Fog;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TObjectPtr<UMaterialParameterCollection> WeatherParameters;
    UPROPERTY(EditAnywhere) float DayLux = 80000.f;
    UPROPERTY(EditAnywhere) float MoonLux = 0.3f;
    UFUNCTION(BlueprintImplementableEvent) void EnvironmentUpdated(float Hour, float Rain, float Wetness, float Snow);
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    FTimerHandle UpdateTimer;
    void UpdateEnvironment();
};
