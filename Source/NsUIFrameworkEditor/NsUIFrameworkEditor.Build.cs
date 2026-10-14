using UnrealBuildTool;

public class NsUIFrameworkEditor : ModuleRules
{
    public NsUIFrameworkEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "NsUIFramework", "UMG" });
        PrivateDependencyModuleNames.AddRange(new[] { "CoreUObject", "Engine", "Slate", "SlateCore", "InputCore" });
    }
}
