#define FASTSIMD_MAX_FEATURE_SET SSE2
#include <FastSIMD/Utility/ArchDetect.h>
#include <FastSIMD/FastSIMD_FastNoise_config.h>
#include "../dispatch/impl/DispatchClassImpl.h"
// Upstream simplex kernels use local constants that shadow namespace constants.
// Keep Unreal's shadow warnings enabled outside this third-party translation unit.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4459)
#endif
#include "../../FastNoise2/src/FastSIMD_Build.inl"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
