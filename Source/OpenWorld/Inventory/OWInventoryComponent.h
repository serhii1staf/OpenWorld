#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OWInventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FOWItemStack
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame) FName Item;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame) int32 Count = 0;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOWInventoryChanged);

UCLASS(ClassGroup=(OpenWorld), meta=(BlueprintSpawnableComponent))
class OPENWORLD_API UOWInventoryComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UOWInventoryComponent();
    UPROPERTY(BlueprintAssignable) FOWInventoryChanged OnChanged;
    UPROPERTY(EditDefaultsOnly) TArray<FOWItemStack> InitialItems;
    UPROPERTY(EditDefaultsOnly) int64 InitialMoney = 0;
    UFUNCTION(BlueprintPure) int32 Count(FName Item) const;
    UFUNCTION(BlueprintPure) int64 GetMoney() const { return Money; }
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly) bool Add(FName Item, int32 Quantity);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly) bool Remove(FName Item, int32 Quantity);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly) bool Credit(int64 Amount);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly) bool Spend(int64 Amount);
    const TArray<FOWItemStack>& GetItems() const { return Items; }
    bool Restore(const TArray<FOWItemStack>& NewItems, int64 NewMoney);
    static bool ValidateSnapshot(const TArray<FOWItemStack>& NewItems, int64 NewMoney);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
protected:
    virtual void BeginPlay() override;
private:
    UPROPERTY(ReplicatedUsing=Changed, SaveGame) TArray<FOWItemStack> Items;
    UPROPERTY(ReplicatedUsing=Changed, SaveGame) int64 Money = 0;
    UFUNCTION() void Changed() { OnChanged.Broadcast(); }
};
