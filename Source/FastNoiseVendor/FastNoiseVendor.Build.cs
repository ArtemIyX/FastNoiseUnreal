using System.IO;
using UnrealBuildTool;

public class FastNoiseVendor : ModuleRules
{
    public FastNoiseVendor(ReadOnlyTargetRules Target) : base(Target)
    {
        if ((Target.Platform != UnrealTargetPlatform.Win64 && Target.Platform != UnrealTargetPlatform.Linux) || Target.Architecture == UnrealArch.Arm64)
            throw new BuildException("FastNoiseVendor currently supports Win64 and Linux x64 only.");
        PCHUsage = PCHUsageMode.NoPCHs;
        bUseUnity = false;
        bUseRTTI = true;
        IWYUSupport = IWYUSupport.None;
        FPSemantics = FPSemanticsMode.Precise;
        PublicDependencyModuleNames.Add("Core");
        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "ThirdParty", "FastNoise2", "include"));
        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "ThirdParty", "FastSIMD", "include"));
        PrivateDefinitions.Add("FASTNOISE_STATIC_LIB=1");
        PrivateDefinitions.Add("FASTSIMD_STATIC_LIB=1");
        PrivateDefinitions.Add("FASTSIMD_LIBRARY_NAME=FastSIMD_FastNoise");
    }
}
