using System.IO;
using System.Linq;
using EpicGames.Core;
using UnrealBuildBase;
using UnrealBuildTool;

public class NsUIFrameworkInput : ModuleRules
{
    public NsUIFrameworkInput(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "NsUIFramework" });
        PrivateDependencyModuleNames.AddRange(new[] { "SlateCore", "Slate" });

        // NsInput is optional. Link it only when the plugin is present, so NsUIFramework compiles alone
        // The search of the engine and the project does not see the additional plugin folders of the project, so a studio plugin next to NsUIFramework counts too
        DirectoryReference ProjectDirectory = Target.ProjectFile != null ? Target.ProjectFile.Directory : null;
        bool bWithNsInput = Plugins.ReadAvailablePlugins(Unreal.EngineDirectory, ProjectDirectory, null).Any(Plugin => Plugin.Name == "NsInput")
            || File.Exists(Path.Combine(PluginDirectory, "..", "NsInput", "NsInput.uplugin"));
        if (bWithNsInput)
        {
            PrivateDependencyModuleNames.Add("NsInput");
        }
        PublicDefinitions.Add("WITH_NSINPUT=" + (bWithNsInput ? "1" : "0"));
    }
}
