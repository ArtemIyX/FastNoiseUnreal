#include "FastNoiseGenerator.h"
#include "FastNoiseNative.h"

UFastNoiseGenerator::~UFastNoiseGenerator() = default;

UFastNoiseGenerator* UFastNoiseGenerator::CreatePerlin(FString& Error)
{
	UFastNoiseGenerator* Object = NewObject<UFastNoiseGenerator>();
	Object->Native = FFastNoiseNative::CreatePerlin(Error);
	return Object->Native ? Object : nullptr;
}
UFastNoiseGenerator* UFastNoiseGenerator::CreateFbm(int32 Octaves, float Gain, float Lacunarity, FString& Error)
{
	UFastNoiseGenerator* Object = NewObject<UFastNoiseGenerator>();
	Object->Native = FFastNoiseNative::CreateFbm(Octaves, Gain, Lacunarity, Error);
	return Object->Native ? Object : nullptr;
}
UFastNoiseGenerator* UFastNoiseGenerator::CreateEncoded(const FString& Encoded, FString& Error)
{
	UFastNoiseGenerator* Object = NewObject<UFastNoiseGenerator>();
	Object->Native = FFastNoiseNative::CreateEncoded(Encoded, Error);
	return Object->Native ? Object : nullptr;
}
bool UFastNoiseGenerator::Sample2D(
	const TArray<FVector2D>& Positions, int32 Seed, TArray<float>& Values, FString& Error) const
{
	Values.Reset();
	if (!Native)
	{
		Error = TEXT("Generator is not initialized");
		return false;
	}
	TArray<float> X, Y;
	X.Reserve(Positions.Num());
	Y.Reserve(Positions.Num());
	for (const FVector2D& Position : Positions)
	{
		const float PX = float(Position.X), PY = float(Position.Y);
		if (!FMath::IsFinite(PX) || !FMath::IsFinite(PY))
		{
			Error = TEXT("Invalid position");
			return false;
		}
		X.Add(PX);
		Y.Add(PY);
	}
	Values.SetNumUninitialized(Positions.Num());
	if (!Native->Sample2D(Positions.Num(), X.GetData(), Y.GetData(), Seed, Values.GetData(), Error))
	{
		Values.Reset();
		return false;
	}
	return true;
}
bool UFastNoiseGenerator::Sample3D(
	const TArray<FVector>& Positions, int32 Seed, TArray<float>& Values, FString& Error) const
{
	Values.Reset();
	if (!Native)
	{
		Error = TEXT("Generator is not initialized");
		return false;
	}
	TArray<float> X, Y, Z;
	X.Reserve(Positions.Num());
	Y.Reserve(Positions.Num());
	Z.Reserve(Positions.Num());
	for (const FVector& Position : Positions)
	{
		const float PX = float(Position.X), PY = float(Position.Y), PZ = float(Position.Z);
		if (!FMath::IsFinite(PX) || !FMath::IsFinite(PY) || !FMath::IsFinite(PZ))
		{
			Error = TEXT("Invalid position");
			return false;
		}
		X.Add(PX);
		Y.Add(PY);
		Z.Add(PZ);
	}
	Values.SetNumUninitialized(Positions.Num());
	if (!Native->Sample3D(Positions.Num(), X.GetData(), Y.GetData(), Z.GetData(), Seed, Values.GetData(), Error))
	{
		Values.Reset();
		return false;
	}
	return true;
}
