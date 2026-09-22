#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OWWeaponComponent.generated.h"
class UOWWeaponDefinition;
class UOWInventoryComponent;
UCLASS(ClassGroup=(OpenWorld), meta=(BlueprintSpawnableComponent))
class OPENWORLD_API UOWWeaponComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UOWWeaponComponent();
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UOWWeaponDefinition> Definition;
    UPROPERTY(Replicated, BlueprintReadOnly, SaveGame) int32 Magazine = 0;
    UPROPERTY(Replicated, BlueprintReadOnly) bool bReloading = false;
    void StartFire();
    void StopFire();
    void Reload();
    void CancelActions();
    bool RestoreMagazine(int32 Value);
    UFUNCTION(NetMulticast, Unreliable) void ShotFX(FVector_NetQuantize Origin, FVector_NetQuantizeNormal Direction);
    UFUNCTION(BlueprintImplementableEvent) void OnShotFX(FVector Origin, FVector Direction);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    FTimerHandle FireTimer, ReloadTimer;
    double NextShotAt = 0;
    void FireOne();
    void FinishReload();
    UOWInventoryComponent* Inventory() const;
};
