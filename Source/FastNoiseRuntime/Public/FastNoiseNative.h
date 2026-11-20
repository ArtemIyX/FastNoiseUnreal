#pragma once

#include "CoreMinimal.h"

// Exclusive ownership: create, use, and destroy a generator on its owning thread.
class FASTNOISERUNTIME_API FFastNoiseNative
{
public:
	~FFastNoiseNative();
	FFastNoiseNative(const FFastNoiseNative&) = delete;
	FFastNoiseNative& operator=(const FFastNoiseNative&) = delete;

	static TUniquePtr<FFastNoiseNative> CreatePerlin(FString& OutError);
	static TUniquePtr<FFastNoiseNative> CreateValue(FString& OutError);
	static TUniquePtr<FFastNoiseNative> CreateSimplex(FString& OutError);
	static TUniquePtr<FFastNoiseNative> CreateFbm(int32 Octaves, float Gain, float Lacunarity, FString& OutError);
	static TUniquePtr<FFastNoiseNative> CreateEncoded(const FString& Encoded, FString& OutError);
	bool Sample2D(int32 Count, const float* X, const float* Y, int32 Seed, float* Output, FString& OutError) const;
	bool Sample3D(int32 Count, const float* X, const float* Y, const float* Z, int32 Seed, float* Output,
		FString& OutError) const;

private:
	FFastNoiseNative() = default;
	static TUniquePtr<FFastNoiseNative> Create(
		bool bFbm, int32 Octaves, float Gain, float Lacunarity, const TCHAR* SourceName, const FString* Encoded, FString& OutError);
	void* Root = nullptr;
	void* Source = nullptr;
	uint32 OwnerThread = 0;
	double MaximumPosition = 100000000.0;
};
