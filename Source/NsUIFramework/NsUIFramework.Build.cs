using UnrealBuildTool;

public class NsUIFramework : ModuleRules
{
    public NsUIFramework(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Public headers use UObjects, actors, local player subsystems, UMG widgets, Slate enums, keys, and developer settings
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "SlateCore", "UMG", "DeveloperSettings", "EnhancedInput" });
        PrivateDependencyModuleNames.AddRange(new[] { "Slate" });
    }
}
