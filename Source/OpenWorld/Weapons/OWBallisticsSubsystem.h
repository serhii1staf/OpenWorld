#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OWBallisticsSubsystem.generated.h"
class AController;
struct FOWBullet
{
    FVector Position = FVector::ZeroVector;
    FVector Velocity = FVector::ZeroVector;
    float Damage = 0.f;
    float Age = 0.f;
    uint8 Bounces = 0;
    TWeakObjectPtr<AActor> Source;
    TWeakObjectPtr<AController> Controller;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOWBulletImpact, const FHitResult&, Hit);
UCLASS()
class OPENWORLD_API UOWBallisticsSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;
    bool Fire(AActor* Source, AController* Controller, FVector Origin, FVector Velocity, float Damage);
    void Clear() { Bullets.Reset(); Accumulator = 0.f; }
    UPROPERTY(BlueprintAssignable) FOWBulletImpact OnImpact;
    int32 GetActiveCount() const { return Bullets.Num(); }
private:
    TArray<FOWBullet> Bullets;
    float Accumulator = 0.f;
    void Step(float Delta);
};
