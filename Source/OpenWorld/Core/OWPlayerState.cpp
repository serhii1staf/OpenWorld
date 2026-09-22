#include "Core/OWPlayerState.h"
#include "Inventory/OWInventoryComponent.h"
#include "Missions/OWMissionComponent.h"
AOWPlayerState::AOWPlayerState()
{
    Inventory = CreateDefaultSubobject<UOWInventoryComponent>(TEXT("Inventory"));
    Missions = CreateDefaultSubobject<UOWMissionComponent>(TEXT("Missions"));
}
void AOWPlayerState::CopyProperties(APlayerState* Other)
{
    Super::CopyProperties(Other);
    if (AOWPlayerState* P = Cast<AOWPlayerState>(Other))
    {
        P->Inventory->Restore(Inventory->GetItems(), Inventory->GetMoney());
        P->Missions->Restore(Missions->Progress);
    }
}
