#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FastNoiseNative.h"
#include "FastNoiseGenerator.generated.h"

UCLASS(BlueprintType)
class FASTNOISERUNTIME_API UFastNoiseGenerator : public UObject
{
	GENERATED_BODY()
public:
	virtual ~UFastNoiseGenerator() override;

	UFUNCTION(BlueprintCallable, Category = "FastNoise2")
	static UFastNoiseGenerator* CreatePerlin(FString& Error);
	UFUNCTION(BlueprintCallable, Category = "FastNoise2")
	static UFastNoiseGenerator* CreateFbm(int32 Octaves, float Gain, float Lacunarity, FString& Error);
	UFUNCTION(BlueprintCallable, Category = "FastNoise2")
	static UFastNoiseGenerator* CreateEncoded(const FString& Encoded, FString& Error);
	UFUNCTION(BlueprintCallable, Category = "FastNoise2")
	bool Sample2D(const TArray<FVector2D>& Positions, int32 Seed, TArray<float>& Values, FString& Error) const;
	UFUNCTION(BlueprintCallable, Category = "FastNoise2")
	bool Sample3D(const TArray<FVector>& Positions, int32 Seed, TArray<float>& Values, FString& Error) const;

private:
	TUniquePtr<FFastNoiseNative> Native;
};
