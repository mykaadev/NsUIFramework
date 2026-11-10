using System.IO;
using System.Linq;
using EpicGames.Core;
using UnrealBuildBase;
using UnrealBuildTool;

public class NsUIFrameworkTween : ModuleRules
{
    public NsUIFrameworkTween(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UMG", "NsUIFramework" });

        // NsTween is optional. Link it only when the plugin is present, so NsUIFramework compiles alone
        // The search of the engine and the project does not see the additional plugin folders of the project, so a studio plugin next to NsUIFramework counts too
        DirectoryReference ProjectDirectory = Target.ProjectFile != null ? Target.ProjectFile.Directory : null;
        bool bWithNsTween = Plugins.ReadAvailablePlugins(Unreal.EngineDirectory, ProjectDirectory, null).Any(Plugin => Plugin.Name == "NsTween")
            || File.Exists(Path.Combine(PluginDirectory, "..", "NsTween", "NsTween.uplugin"));
        if (bWithNsTween)
        {
            PrivateDependencyModuleNames.Add("NsTween");
        }
        PublicDefinitions.Add("WITH_NSTWEEN=" + (bWithNsTween ? "1" : "0"));
    }
}
