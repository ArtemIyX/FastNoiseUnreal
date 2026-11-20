#include "FastNoiseNative.h"
#include "FastNoise/FastNoise_C.h"
#include "FastSIMD/Utility/FeatureEnums.h"
#include "HAL/PlatformTLS.h"
#include "Misc/ScopeLock.h"
#include <cfloat>

namespace
{
FCriticalSection NodeMutex;
FString Normalize(const char* Name)
{
	FString Result;
	if (Name)
		for (const char* C = Name; *C; ++C)
		{
			if (*C >= 'A' && *C <= 'Z')
				Result.AppendChar(TCHAR(*C + ('a' - 'A')));
			else if ((*C >= 'a' && *C <= 'z') || (*C >= '0' && *C <= '9'))
				Result.AppendChar(TCHAR(*C));
		}
	return Result;
}
int32 FindNode(const TCHAR* Name)
{
	for (int32 Index = 0; Index < fnGetMetadataCount(); ++Index)
		if (Normalize(fnGetMetadataName(Index)) == Name)
			return Index;
	return INDEX_NONE;
}
int32 FindField(int32 Id, int32 Count, const TCHAR* Name, const char* (*GetName)(int, int))
{
	for (int32 Index = 0; Index < Count; ++Index)
		if (Normalize(GetName(Id, Index)) == Name)
			return Index;
	return INDEX_NONE;
}
}

FFastNoiseNative::~FFastNoiseNative()
{
	check(OwnerThread == FPlatformTLS::GetCurrentThreadId());
	FScopeLock Lock(&NodeMutex);
	if (Root)
		fnDeleteNodeRef(Root);
	if (Source)
		fnDeleteNodeRef(Source);
}
TUniquePtr<FFastNoiseNative> FFastNoiseNative::CreatePerlin(FString& Error)
{
	return Create(false, 1, 0.f, 1.f, nullptr, Error);
}
TUniquePtr<FFastNoiseNative> FFastNoiseNative::CreateFbm(int32 Octaves, float Gain, float Lacunarity, FString& Error)
{
	return Create(true, Octaves, Gain, Lacunarity, nullptr, Error);
}
TUniquePtr<FFastNoiseNative> FFastNoiseNative::CreateEncoded(const FString& Encoded, FString& Error)
{
	return Create(false, 1, 0.f, 1.f, &Encoded, Error);
}
TUniquePtr<FFastNoiseNative> FFastNoiseNative::Create(
	bool bFbm, int32 Octaves, float Gain, float Lacunarity, const FString* Encoded, FString& Error)
{
	Error.Reset();
	if (bFbm
		&& (Octaves < 1 || Octaves > 16 || !FMath::IsFinite(Gain) || Gain < 0.f || Gain > 1.f
			|| !FMath::IsFinite(Lacunarity) || Lacunarity < 1.f))
	{
		Error = TEXT("InvalidNoiseInput: invalid fBm settings");
		return nullptr;
	}
	double MaxScale = 1.0;
	if (bFbm)
	{
		for (int32 Octave = 1; Octave < Octaves; ++Octave)
		{
			MaxScale *= double(Lacunarity);
			if (!FMath::IsFinite(MaxScale) || MaxScale > double(FLT_MAX))
			{
				Error = TEXT("InvalidNoiseInput: fBm octave scale is too large");
				return nullptr;
			}
		}
	}
	TUniquePtr<FFastNoiseNative> Result(new FFastNoiseNative);
	Result->OwnerThread = FPlatformTLS::GetCurrentThreadId();
	Result->MaximumPosition /= MaxScale;
	{
		FScopeLock Lock(&NodeMutex);
		const unsigned Level = unsigned(FastSIMD::FeatureSet::SSE2);
		if (Encoded)
		{
			if (!Encoded->IsEmpty())
				Result->Root = fnNewFromEncodedNodeTree(TCHAR_TO_UTF8(**Encoded), Level);
		}
		else
		{
			const int32 PerlinId = FindNode(TEXT("perlin"));
			if (PerlinId != INDEX_NONE)
				Result->Source = fnNewFromMetadata(PerlinId, Level);
			if (!bFbm)
			{
				Result->Root = Result->Source;
				Result->Source = nullptr;
			}
			else if (Result->Source)
			{
				const int32 FbmId = FindNode(TEXT("fractalfbm"));
				if (FbmId != INDEX_NONE)
					Result->Root = fnNewFromMetadata(FbmId, Level);
				if (Result->Root)
				{
					const int32 SourceIndex = FindField(
						FbmId, fnGetMetadataNodeLookupCount(FbmId), TEXT("source"), fnGetMetadataNodeLookupName);
					const int32 Octave =
						FindField(FbmId, fnGetMetadataVariableCount(FbmId), TEXT("octaves"), fnGetMetadataVariableName);
					const int32 Lac = FindField(
						FbmId, fnGetMetadataVariableCount(FbmId), TEXT("lacunarity"), fnGetMetadataVariableName);
					const int32 GainIndex =
						FindField(FbmId, fnGetMetadataHybridCount(FbmId), TEXT("gain"), fnGetMetadataHybridName);
					if (SourceIndex == INDEX_NONE || Octave == INDEX_NONE || Lac == INDEX_NONE
						|| GainIndex == INDEX_NONE || !fnSetNodeLookup(Result->Root, SourceIndex, Result->Source)
						|| !fnSetVariableIntEnum(Result->Root, Octave, Octaves)
						|| !fnSetHybridFloat(Result->Root, GainIndex, Gain)
						|| !fnSetVariableFloat(Result->Root, Lac, Lacunarity))
						Error = TEXT("BackendUnavailable: could not configure fBm");
				}
			}
		}
		if (!Result->Root || fnGetActiveFeatureSet(Result->Root) != Level)
			Error = TEXT("BackendUnavailable: could not create requested SSE2 node");
	}
	if (!Error.IsEmpty())
		return nullptr;
	return Result;
}
bool FFastNoiseNative::Sample2D(
	int32 Count, const float* X, const float* Y, int32 Seed, float* Output, FString& Error) const
{
	Error.Reset();
	if (OwnerThread != FPlatformTLS::GetCurrentThreadId() || Count < 0 || (Count && (!X || !Y || !Output)))
	{
		Error = TEXT("InvalidNoiseInput: invalid batch or owning thread");
		return false;
	}
	for (int32 Index = 0; Index < Count; ++Index)
		if (!FMath::IsFinite(X[Index]) || !FMath::IsFinite(Y[Index]) || FMath::Abs(double(X[Index])) > MaximumPosition
			|| FMath::Abs(double(Y[Index])) > MaximumPosition)
		{
			Error = TEXT("InvalidNoiseInput: position exceeds safe lattice domain");
			return false;
		}
	if (Count)
		fnGenPositionArray2D(Root, Output, Count, X, Y, 0.f, 0.f, Seed, nullptr);
	for (int32 Index = 0; Index < Count; ++Index)
		if (!FMath::IsFinite(Output[Index]))
		{
			Error = TEXT("InvalidNoiseInput: non-finite result");
			return false;
		}
	return true;
}
bool FFastNoiseNative::Sample3D(
	int32 Count, const float* X, const float* Y, const float* Z, int32 Seed, float* Output, FString& Error) const
{
	Error.Reset();
	if (OwnerThread != FPlatformTLS::GetCurrentThreadId() || Count < 0 || (Count && (!X || !Y || !Z || !Output)))
	{
		Error = TEXT("InvalidNoiseInput: invalid batch or owning thread");
		return false;
	}
	for (int32 Index = 0; Index < Count; ++Index)
		if (!FMath::IsFinite(X[Index]) || !FMath::IsFinite(Y[Index]) || !FMath::IsFinite(Z[Index])
			|| FMath::Abs(double(X[Index])) > MaximumPosition || FMath::Abs(double(Y[Index])) > MaximumPosition
			|| FMath::Abs(double(Z[Index])) > MaximumPosition)
		{
			Error = TEXT("InvalidNoiseInput: position exceeds safe lattice domain");
			return false;
		}
	if (Count)
		fnGenPositionArray3D(Root, Output, Count, X, Y, Z, 0.f, 0.f, 0.f, Seed, nullptr);
	for (int32 Index = 0; Index < Count; ++Index)
		if (!FMath::IsFinite(Output[Index]))
		{
			Error = TEXT("InvalidNoiseInput: non-finite result");
			return false;
		}
	return true;
}
