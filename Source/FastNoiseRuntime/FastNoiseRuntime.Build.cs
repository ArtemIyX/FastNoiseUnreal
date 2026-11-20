using UnrealBuildTool;

public class FastNoiseRuntime : ModuleRules
{
    public FastNoiseRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.Add("FastNoiseVendor");
    }
}
