# FastNoise

Source-built FastNoise2 1.1.1 integration for Unreal Engine 5.7. The FastNoiseVendor runtime module includes FastNoise2 and FastSIMD source and builds through UnrealBuildTool; no external FastNoise DLL, import library, or CMake invocation is required. The module currently supports Win64 and Linux x86-64 with an SSE2 dispatch backend and precise floating point semantics.

Pinned source revisions and upstream licenses are in `Source/FastNoiseVendor/Private/ThirdParty`. FastNoise2: `c35d6041ce7efc05bc84e26d762f6a38d2dcc71e`; FastSIMD: `f7e9ce6fca475731c0a3c115ec29dcfd36886b2a`.

## API

`FFastNoiseNative` is a non-UObject, worker-owned C++ generator for Perlin, fBm, and encoded node trees. Create, sample, and destroy a generator on the same thread; do not share a mutable generator between workers. The `FastNoiseRuntime` UObject `UFastNoiseGenerator` exposes corresponding factories and 2D/3D position sampling to Blueprint on the game thread. Existing assets using the previous wrapper must be migrated.

## Verification

Build the plugin through the host Unreal project and run `FastNoise2.Source` and `VoxelLand.M1` Automation on Win64 and Linux x64. Native scalar is not supported; VoxelLand retains its independent scalar VM path. Capture and compare new SSE2 noise arrays using VoxelLand's `Docs/M1_Verification.md`; FastNoise2 1.1.1 is a version change and old noise goldens must not be assumed valid.
