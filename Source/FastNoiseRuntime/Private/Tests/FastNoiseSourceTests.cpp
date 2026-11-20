#include "FastNoiseNative.h"
#include "FastNoiseGenerator.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Async/Async.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFastNoiseSourceSmoke, "FastNoise2.Source.Smoke",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FFastNoiseSourceSmoke::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("runtime module loaded"), FModuleManager::Get().IsModuleLoaded(TEXT("FastNoiseRuntime")));
	struct FResult
	{
		FString Error;
		bool bOffGameThread = false;
		bool bWrongThreadRejected = false;
		bool bInvalidFbmRejected = false;
		bool bInvalidEncodedRejected = false;
		bool bEncodedCreated = false;
		float Perlin2[3] = {};
		float Perlin3[3] = {};
		float Fbm2[3] = {};
		float Fbm3[3] = {};
	};
	const FResult Result = Async(EAsyncExecution::ThreadPool, []() {
		FResult R;
		R.bOffGameThread = !IsInGameThread();
		FString Error;
		TUniquePtr<FFastNoiseNative> Perlin = FFastNoiseNative::CreatePerlin(Error);
		if (!Perlin)
		{
			R.Error = Error;
			return R;
		}
		TUniquePtr<FFastNoiseNative> Fbm = FFastNoiseNative::CreateFbm(3, 0.5f, 2.f, Error);
		if (!Fbm)
		{
			R.Error = Error;
			return R;
		}
		const float X[] = {-1.f, 0.f, 1.f}, Y[] = {2.f, -3.f, 0.f}, Z[] = {0.f, -1.f, 2.f};
		if (!Perlin->Sample2D(3, X, Y, -19, R.Perlin2, Error) || !Perlin->Sample3D(3, X, Y, Z, -19, R.Perlin3, Error)
			|| !Fbm->Sample2D(3, X, Y, -19, R.Fbm2, Error) || !Fbm->Sample3D(3, X, Y, Z, -19, R.Fbm3, Error))
		{
			R.Error = Error;
			return R;
		}
		FFastNoiseNative* Raw = Perlin.Get();
		R.bWrongThreadRejected = Async(EAsyncExecution::Thread, [Raw]() {
			float Out = 0.f;
			const float P = 0.f;
			FString WrongError;
			return !Raw->Sample2D(1, &P, &P, 0, &Out, WrongError);
		}).Get();
		R.bInvalidFbmRejected = !FFastNoiseNative::CreateFbm(0, 0.5f, 2.f, Error);
		R.bInvalidEncodedRejected = !FFastNoiseNative::CreateEncoded(TEXT(""), Error);
		TUniquePtr<FFastNoiseNative> Encoded = FFastNoiseNative::CreateEncoded(
			TEXT(
				"E@BBZEG@BD8JFgIECArXIzwECiQIw/UoPwkuAAE@BJDQAH@BC@AIEAJBw@ABZEED0KV78YZmZmPwQDmpkZPwsAAIA/HAMAAHBCBA=="),
			Error);
		R.bEncodedCreated = Encoded.IsValid();
		return R;
	}).Get();
	TestTrue(TEXT("native work off game thread"), Result.bOffGameThread);
	TestTrue(TEXT("native creation and sampling"), Result.Error.IsEmpty());
	if (!Result.Error.IsEmpty())
		AddError(Result.Error);
	TestTrue(TEXT("wrong thread rejected"), Result.bWrongThreadRejected);
	TestTrue(TEXT("invalid fBm rejected"), Result.bInvalidFbmRejected);
	TestTrue(TEXT("invalid encoding rejected"), Result.bInvalidEncodedRejected);
	TestTrue(TEXT("upstream encoded tree created"), Result.bEncodedCreated);
	for (int32 Index = 0; Index < 3; ++Index)
		TestTrue(TEXT("finite output"), FMath::IsFinite(Result.Perlin2[Index]) && FMath::IsFinite(Result.Perlin3[Index])
											&& FMath::IsFinite(Result.Fbm2[Index])
											&& FMath::IsFinite(Result.Fbm3[Index]));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFastNoiseBlueprintSmoke, "FastNoise2.Source.Blueprint",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FFastNoiseBlueprintSmoke::RunTest(const FString& Parameters)
{
	if (!IsInGameThread())
	{
		AddError(TEXT("Blueprint smoke must run on the game thread"));
		return false;
	}
	FString Error;
	UFastNoiseGenerator* Perlin = UFastNoiseGenerator::CreatePerlin(Error);
	UFastNoiseGenerator* Fbm = UFastNoiseGenerator::CreateFbm(3, 0.5f, 2.f, Error);
	TestNotNull(TEXT("Perlin generator"), Perlin);
	TestNotNull(TEXT("fBm generator"), Fbm);
	if (Perlin && Fbm)
	{
		TArray<float> Output;
		const TArray<FVector2D> Points2D = {FVector2D(-1.0, 2.0), FVector2D(0.0, 0.0)};
		const TArray<FVector> Points3D = {FVector(-1.0, 2.0, 3.0)};
		TestTrue(TEXT("Perlin Blueprint 2D"), Perlin->Sample2D(Points2D, -19, Output, Error));
		TestEqual(TEXT("2D count"), Output.Num(), 2);
		TestTrue(TEXT("fBm Blueprint 3D"), Fbm->Sample3D(Points3D, -19, Output, Error));
		TestEqual(TEXT("3D count"), Output.Num(), 1);
	}
	return true;
}

#endif
