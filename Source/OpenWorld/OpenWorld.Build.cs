using UnrealBuildTool;
public class OpenWorld : ModuleRules
{
    public OpenWorld(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore", "NetCore",
            "AIModule", "NavigationSystem", "GameplayTasks", "ChaosVehicles",
            "PhysicsCore", "UMG", "Slate", "SlateCore", "DeveloperSettings"
        });
    }
}
