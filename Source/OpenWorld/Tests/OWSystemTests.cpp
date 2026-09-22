#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Core/OWPlayerState.h"
#include "Inventory/OWInventoryComponent.h"
#include "Missions/OWMissionComponent.h"
#include "SaveGame/OWWorldStateSubsystem.h"
namespace
{
    struct FTestWorld
    {
        UWorld* World;
        FTestWorld()
        {
            World = UWorld::CreateWorld(EWorldType::Game, false);
            World->InitializeNewWorld(UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false).SetTransactional(false));
        }
        ~FTestWorld() { World->DestroyWorld(false); }
    };
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOWInventoryValidation, "OpenWorld.Inventory.Validation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FOWInventoryValidation::RunTest(const FString&)
{
    TArray<FOWItemStack> Items; FOWItemStack S; S.Item = "Ammo.9mm"; S.Count = 30; Items.Add(S);
    TestTrue(TEXT("Valid stack"), UOWInventoryComponent::ValidateSnapshot(Items, 100));
    Items.Add(S); TestFalse(TEXT("Duplicate IDs"), UOWInventoryComponent::ValidateSnapshot(Items, 100));
    Items.RemoveAt(1); Items[0].Count = -1; TestFalse(TEXT("Negative count"), UOWInventoryComponent::ValidateSnapshot(Items,100));
    Items[0].Count = 10000; TestFalse(TEXT("Stack overflow"), UOWInventoryComponent::ValidateSnapshot(Items,100));
    Items[0].Count = 30; TestFalse(TEXT("Negative money"), UOWInventoryComponent::ValidateSnapshot(Items,-1));
    TestFalse(TEXT("Money cap"), UOWInventoryComponent::ValidateSnapshot(Items,1000000001LL));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOWInventoryTransactions, "OpenWorld.Inventory.Transactions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FOWInventoryTransactions::RunTest(const FString&)
{
    FTestWorld W; AOWPlayerState* P = W.World->SpawnActor<AOWPlayerState>();
    TestTrue(TEXT("Add"),P->Inventory->Add("Ammo",50));
    TestFalse(TEXT("Reject overdraft"),P->Inventory->Remove("Ammo",51));
    TestEqual(TEXT("No partial mutation"),P->Inventory->Count("Ammo"),50);
    TestTrue(TEXT("Remove exact count"),P->Inventory->Remove("Ammo",50));
    TestEqual(TEXT("Empty"),P->Inventory->GetItems().Num(),0);
    TestTrue(TEXT("Credit"),P->Inventory->Credit(100));
    TestFalse(TEXT("Reject overspend"),P->Inventory->Spend(101));
    TestEqual(TEXT("Money retained"),P->Inventory->GetMoney(),static_cast<int64>(100));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOWMissionProgression, "OpenWorld.Missions.Progression", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FOWMissionProgression::RunTest(const FString&)
{
    FTestWorld W; AOWPlayerState* P = W.World->SpawnActor<AOWPlayerState>();
    UOWMissionDefinition* D = NewObject<UOWMissionDefinition>(P); D->MissionId = "Delivery"; D->Reward = 250;
    FOWObjective O; O.Event = "Pickup"; O.Required = 2; D->Objectives.Add(O); O.Event = "Deliver"; O.Required = 1; D->Objectives.Add(O);
    P->Missions->Definitions.Add(D);
    TestTrue(TEXT("Start"),P->Missions->Start("Delivery"));
    TestFalse(TEXT("Cannot start twice"),P->Missions->Start("Delivery"));
    P->Missions->Emit("Deliver"); TestEqual(TEXT("Order enforced"),P->Missions->Progress[0].Objective,0);
    P->Missions->Emit("Pickup",2); TestEqual(TEXT("First objective complete"),P->Missions->Progress[0].Objective,1);
    P->Missions->Emit("Deliver"); TestTrue(TEXT("Complete"),P->Missions->Progress[0].bComplete);
    P->Missions->Emit("Deliver"); TestEqual(TEXT("Reward once"),P->Inventory->GetMoney(),static_cast<int64>(250));
    TArray<FOWMissionProgress> Bad = P->Missions->Progress; Bad[0].Objective = 99;
    TestFalse(TEXT("Reject invalid restore"),P->Missions->Restore(Bad));
    TestEqual(TEXT("State retained"),P->Missions->Progress[0].Objective,2);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOWWorldValidation, "OpenWorld.Persistence.Validation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FOWWorldValidation::RunTest(const FString&)
{
    FOWWorldRecord R; R.Id = FGuid::NewGuid(); R.Kind = "Vehicle"; R.Transform = FTransform::Identity;
    TArray<FOWWorldRecord> Records; Records.Add(R);
    TestTrue(TEXT("Valid record"),UOWWorldStateSubsystem::ValidateSnapshot(Records));
    Records.Add(R); TestFalse(TEXT("Duplicate GUID rejected"),UOWWorldStateSubsystem::ValidateSnapshot(Records));
    Records.RemoveAt(1); Records[0].Id.Invalidate(); TestFalse(TEXT("Missing GUID rejected"),UOWWorldStateSubsystem::ValidateSnapshot(Records));
    return true;
}
#endif
