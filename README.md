<div align="center">

<img width="1280" height="640" alt="Header" src="https://github.com/user-attachments/assets/4afe3fc9-2f67-4dbf-a468-78e16dd3a8c9" />

# FastNoise for Unreal Engine

**Source-built FastNoise2 noise generation for Unreal Engine.**

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-0E1128?logo=unrealengine&logoColor=white)](https://www.unrealengine.com/)
[![FastNoise2](https://img.shields.io/badge/FastNoise2-1.1.1-3B82F6)](https://github.com/Auburn/FastNoise2)
[![Platforms](https://img.shields.io/badge/Platforms-Win64%20%7C%20Linux%20x64-4285F4)](#requirements)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)

[Features](#features) • [Installation](#installation) • [C++ usage](#c-usage) • [Blueprints](#blueprints) • [License](#license-and-credits)

</div>

FastNoise integrates [Auburn's FastNoise2](https://github.com/Auburn/FastNoise2) and [FastSIMD](https://github.com/Auburn/FastSIMD) into an Unreal Engine plugin. UnrealBuildTool compiles the vendored sources with the plugin, so there is no separate FastNoise library or DLL to build or stage.

## Features

- Create **Perlin**, **fractal Brownian motion (fBm)**, and encoded node-tree generators.
- Sample batches of positions in **2D and 3D** with an explicit seed.
- Use `FFastNoiseNative` from C++ without UObjects, or `UFastNoiseGenerator` from Blueprints.
- Use worker-owned native generators for background generation.
- Compile through UnrealBuildTool with an **SSE2** native noise backend on supported x64 targets.
- Keep upstream code in the `FastNoiseVendor` module and Unreal-facing interfaces in `FastNoiseRuntime`.

## Requirements

- Unreal Engine **5.7** and a C++ project with a supported compiler.
- **Win64 or Linux x86-64**. The current source integration selects SSE2; other architectures and native scalar noise are not supported by this integration.

The plugin vendors FastNoise2 revision `c35d6041ce7efc05bc84e26d762f6a38d2dcc71e` and FastSIMD revision `f7e9ce6fca475731c0a3c115ec29dcfd36886b2a`. See [vendor revisions and integration notes](Source/FastNoiseVendor/Private/ThirdParty/VERSIONS.txt).

## Installation

1. Place this plugin checkout in your project's `Plugins/FastNoise` directory, with `FastNoise.uplugin` at the plugin root.
2. Enable **FastNoise** in the Unreal Editor's Plugins window, or add it to your `.uproject`:

   ```json
   {
     "Name": "FastNoise",
     "Enabled": true
   }
   ```

3. Regenerate project files if needed, then build your project. UnrealBuildTool compiles the vendor sources as part of the plugin.
4. For C++ consumers, add `FastNoiseRuntime` to your module's dependencies in `YourModule.Build.cs`:

   ```csharp
   PrivateDependencyModuleNames.Add("FastNoiseRuntime");
   ```

   Use a public dependency instead if your module's public headers expose FastNoise types.

## C++ usage

Include `FastNoiseNative.h` and create a generator on the thread that will use it. Factories return a `TUniquePtr`; check both creation and sampling errors.

```cpp
#include "FastNoiseNative.h"

bool SampleTerrainNoise(TArray<float>& OutValues, FString& OutError)
{
    // Octaves, gain, and lacunarity configure the fBm generator.
    TUniquePtr<FFastNoiseNative> Noise =
        FFastNoiseNative::CreateFbm(3, 0.5f, 2.0f, OutError);
    if (!Noise)
    {
        return false;
    }

    // Coordinates are supplied as separate arrays of equal length.
    const TArray<float> X = { -1.0f, 0.0f, 1.0f };
    const TArray<float> Y = {  2.0f, 0.0f, 3.0f };
    const TArray<float> Z = {  0.0f, 1.0f, 2.0f };
    constexpr int32 Seed = 42;

    OutValues.SetNumUninitialized(X.Num());
    if (!Noise->Sample3D(X.Num(), X.GetData(), Y.GetData(), Z.GetData(),
                         Seed, OutValues.GetData(), OutError))
    {
        OutValues.Reset();
        return false;
    }

    return true;
}
```

For a 2D batch, call `Sample2D` with X and Y arrays. Use `FFastNoiseNative::CreatePerlin(OutError)` for Perlin noise, or `CreateEncoded(EncodedTree, OutError)` to load an encoded FastNoise2 node tree. `CreateFbm` creates fBm with a Perlin source.

**Thread ownership:** create, sample, and destroy each native generator on the same thread. Give each worker its own generator instead of sharing one across workers. Keep input and output arrays alive for the duration of each call.

## Blueprints

Use the `FastNoise2` Blueprint category to create a `UFastNoiseGenerator` with **Create Perlin**, **Create Fbm**, or **Create Encoded**. Pass a position array and seed to **Sample 2D** or **Sample 3D**. Check the returned generator or success flag and the error output before using the result. Keep a reference to the generator for as long as it is needed.

## Tips

- Use the same generator configuration, seed, positions, and plugin version when reproducing a result.
- Scale your input coordinates to adjust noise frequency; smaller coordinate steps produce broader features.
- fBm accepts 1–16 octaves, gain from 0 to 1, and lacunarity of at least 1. Invalid values are rejected.
- Sampling rejects non-finite coordinates and positions outside the generator's safe lattice domain. Inspect `OutError` when a call fails.
- Different FastNoise versions or CPU backends can produce different results. Capture and compare your own output before changing versions or relying on cross-platform golden values.
- Run the Unreal Automation tests under `FastNoise2.Source` after building the plugin. Projects integrating VoxelLand can also use its `VoxelLand.M1` tests and verification handoff.

## Older version

The previous plugin version based on FastNoise2 v0.10.0-alpha is available on the [v0.10.0-alpha branch](https://github.com/ArtemIyX/FastNoiseUnreal/tree/v0.10.0-alpha). Its Blueprint and C++ APIs differ from this source-built integration; existing assets may need migration.

## License and credits

This plugin is distributed under the [MIT license](LICENSE)

The bundled [FastNoise2](https://github.com/Auburn/FastNoise2) and [FastSIMD](https://github.com/Auburn/FastSIMD) projects are also MIT-licensed by Jordan Peck. Their license texts are included in the [FastNoise2 vendor directory](Source/FastNoiseVendor/Private/ThirdParty/FastNoise2/LICENSE) and [FastSIMD vendor directory](Source/FastNoiseVendor/Private/ThirdParty/FastSIMD/LICENSE).
