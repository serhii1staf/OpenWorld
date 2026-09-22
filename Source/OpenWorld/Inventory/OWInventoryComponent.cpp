#include "Inventory/OWInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
UOWInventoryComponent::UOWInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}
int32 UOWInventoryComponent::Count(FName Item) const
{
    const FOWItemStack* Stack = Items.FindByPredicate([Item](const FOWItemStack& S){ return S.Item == Item; });
    return Stack ? Stack->Count : 0;
}
bool UOWInventoryComponent::Add(FName Item, int32 Quantity)
{
    if (!GetOwner()->HasAuthority() || Item.IsNone() || Quantity <= 0 || Quantity > 9999) return false;
    FOWItemStack* Stack = Items.FindByPredicate([Item](const FOWItemStack& S){ return S.Item == Item; });
    if (Stack)
    {
        if (Stack->Count > 9999 - Quantity) return false;
        Stack->Count += Quantity;
    }
    else
    {
        if (Items.Num() >= 128) return false;
        FOWItemStack NewStack; NewStack.Item = Item; NewStack.Count = Quantity; Items.Add(NewStack);
    }
    Changed(); return true;
}
bool UOWInventoryComponent::Remove(FName Item, int32 Quantity)
{
    if (!GetOwner()->HasAuthority() || Quantity <= 0) return false;
    int32 Index = Items.IndexOfByPredicate([Item](const FOWItemStack& S){ return S.Item == Item; });
    if (Index == INDEX_NONE || Items[Index].Count < Quantity) return false;
    Items[Index].Count -= Quantity;
    if (Items[Index].Count == 0) Items.RemoveAtSwap(Index);
    Changed(); return true;
}
bool UOWInventoryComponent::Credit(int64 Amount)
{
    if (!GetOwner()->HasAuthority() || Amount < 0 || Amount > 1000000000LL - Money) return false;
    Money += Amount; Changed(); return true;
}
bool UOWInventoryComponent::Spend(int64 Amount)
{
    if (!GetOwner()->HasAuthority() || Amount < 0 || Money < Amount) return false;
    Money -= Amount; Changed(); return true;
}
bool UOWInventoryComponent::ValidateSnapshot(const TArray<FOWItemStack>& NewItems, int64 NewMoney)
{
    if (NewMoney < 0 || NewMoney > 1000000000LL || NewItems.Num() > 128) return false;
    TSet<FName> Names;
    for (const FOWItemStack& S : NewItems)
    {
        if (S.Item.IsNone() || S.Count <= 0 || S.Count > 9999 || Names.Contains(S.Item)) return false;
        Names.Add(S.Item);
    }
    return true;
}
bool UOWInventoryComponent::Restore(const TArray<FOWItemStack>& NewItems, int64 NewMoney)
{
    if (!GetOwner()->HasAuthority() || !ValidateSnapshot(NewItems, NewMoney)) return false;
    Items = NewItems; Money = NewMoney; Changed(); return true;
}
void UOWInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const
{
    Super::GetLifetimeReplicatedProps(Out);
    DOREPLIFETIME_CONDITION(UOWInventoryComponent, Items, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(UOWInventoryComponent, Money, COND_OwnerOnly);
}
