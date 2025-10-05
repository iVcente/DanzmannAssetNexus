// Copyright (C) 2025 Dancing Man Games. All Rights Reserved.

#include "DanzmannAssetManager.h"

#include "DanzmannLogDanzmannAssetNexus.h"

UDanzmannAssetManager* UDanzmannAssetManager::Get()
{
	checkf(GEngine != nullptr, TEXT("[%hs] UEngine is invalid."), __FUNCTION__);

	if (UDanzmannAssetManager* DanzmannAssetManager = Cast<UDanzmannAssetManager>(GEngine->AssetManager))
	{
		return DanzmannAssetManager;
	}

	UE_LOG(LogDanzmannAssetNexus, Fatal, TEXT("[%hs] AssetManagerClassName in DefaultEngine.ini is invalid. It must be set to UDanzmannAssetManager."), __FUNCTION__);

	// Fatal error above prevents this from being called
	return NewObject<UDanzmannAssetManager>();
}

int32 UDanzmannAssetManager::UnloadLoadedSoftAssets(const FGameplayTag& BundleToUnload, const bool bExactMatch)
{
	checkf(BundleToUnload != FGameplayTag::EmptyTag, TEXT("UnloadLoadedSoftAssets() only supports non empty BundleToUnload."));
	
	TArray<FGameplayTag> Keys;
	Bundles.GetKeys(Keys);

	int32 NumBundlesRemoved = 0;

	for (const FGameplayTag& Key : Keys)
	{
		if (bExactMatch && BundleToUnload.MatchesTagExact(Key))
		{
			NumBundlesRemoved += Bundles.Remove(Key);
			break;
		}

		if (BundleToUnload.MatchesTag(Key))
		{
			NumBundlesRemoved += Bundles.Remove(Key);
		}
	}

	return NumBundlesRemoved;
}

void UDanzmannAssetManager::DumpLoadedSoftAssets()
{
	checkf(UDanzmannAssetManager::IsInitialized(), TEXT("[%hs] UDanzmannAssetManager isn't initialized."), __FUNCTION__);

	UDanzmannAssetManager* AssetManager = Get();

	UE_LOG(LogDanzmannAssetNexus, Log, TEXT("=========== Danzmann Asset Manager Loaded Soft Assets ==========="));

	for (TPair<FGameplayTag, FDanzmannAssetManagerBundle>& Bundle : AssetManager->Bundles)
	{
		UE_LOG(LogDanzmannAssetNexus, Log, TEXT("\tBundle: %s"), *Bundle.Key.ToString());

		TArray<FString> AssetsNames;
		Bundle.Value.GetAssetsName(AssetsNames);
		
		for (const FString& AssetName : AssetsNames)
		{
			UE_LOG(LogDanzmannAssetNexus, Log, TEXT("\t  -> Asset: %s"), *AssetName);
		}
	}
}

/*
 * CVars
 */

static FAutoConsoleCommand CVarDumpLoadedSoftAssets(
	TEXT("DanzmannAssetManager.DumpLoadedSoftAssets"),
	TEXT("Print a list of currently Loaded soft assets by the Danzmann Asset Manager."),
	FConsoleCommandDelegate::CreateStatic(UDanzmannAssetManager::DumpLoadedSoftAssets),
	ECVF_Cheat
);
