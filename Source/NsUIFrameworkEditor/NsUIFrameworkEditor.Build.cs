using UnrealBuildTool;

public class NsUIFrameworkEditor : ModuleRules
{
    public NsUIFrameworkEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "Slate", "SlateCore", "NsUIFramework" });
    }
}
