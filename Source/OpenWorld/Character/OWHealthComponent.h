#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OWHealthComponent.generated.h"
class UDamageType;
class AController;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOWHealthChanged, float, Health, bool, bDead);
UCLASS(ClassGroup=(OpenWorld), meta=(BlueprintSpawnableComponent))
class OPENWORLD_API UOWHealthComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UOWHealthComponent();
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) float MaxHealth = 100.f;
    UPROPERTY(BlueprintAssignable) FOWHealthChanged OnChanged;
    UFUNCTION(BlueprintPure) float GetHealth() const { return Health; }
    UFUNCTION(BlueprintPure) bool IsDead() const { return Health <= 0.f; }
    bool Restore(float Value);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
private:
    UPROPERTY(ReplicatedUsing=Changed, SaveGame) float Health = 100.f;
    UFUNCTION() void Changed();
    UFUNCTION() void Damaged(AActor* Actor, float Amount, const UDamageType* Type, AController* Instigator, AActor* Causer);
};
