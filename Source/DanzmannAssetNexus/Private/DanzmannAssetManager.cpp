// Copyright (C) 2025 Dancing Man Games. All Rights Reserved.

#include "DanzmannAssetManager.h"

#include "DanzmannLogDanzmannAssetNexus.h"

UDanzmannAssetManager& UDanzmannAssetManager::Get()
{
	checkf(GEngine != nullptr, TEXT("[%hs] UEngine is invalid."), __FUNCTION__);

	if (UDanzmannAssetManager* DanzmannAssetManager = Cast<UDanzmannAssetManager>(GEngine->AssetManager))
	{
		return *DanzmannAssetManager;
	}

	UE_LOG(LogDanzmannAssetNexus, Fatal, TEXT("[%hs] AssetManagerClassName in DefaultEngine.ini is invalid. It must be set to UDanzmannAssetManager."), __FUNCTION__);

	// Fatal error above prevents this from being called
	return *NewObject<UDanzmannAssetManager>();
}

TSharedPtr<FStreamableHandle> UDanzmannAssetManager::PreloadSoftAssets(const TArray<TSoftObjectPtr<UObject>>& AssetsToPreload, FStreamableDelegateWithHandle&& Delegate)
{
	TArray<FSoftObjectPath> AssetsPath;
	AssetsPath.Reserve(AssetsToPreload.Num());

	for (const TSoftObjectPtr<UObject>& AssetToPreload : AssetsToPreload)
	{
		if (!AssetToPreload.IsNull())
		{
			AssetsPath.Add(AssetToPreload.ToSoftObjectPath());
		}
	}
	
	return GetStreamableManager().RequestAsyncLoad(MoveTemp(AssetsPath), MoveTemp(Delegate));
}

TSharedPtr<FStreamableHandle> UDanzmannAssetManager::LoadSoftAssets(const TArray<TSoftObjectPtr<UObject>>& AssetsToLoad, FStreamableDelegateWithHandle&& Delegate, const EDanzmannAssetBundle Bundle)
{
	FStreamableDelegateWithHandle CallerDelegate = MoveTemp(Delegate);
	
	TSharedPtr<FStreamableHandle> StreamableHandle = PreloadSoftAssets(
		AssetsToLoad,
		FStreamableDelegateWithHandle::CreateWeakLambda(
			this,
			[this, Bundle, CallerDelegate]
			(const TSharedPtr<FStreamableHandle>& Handle)
			{
				// Store strong references to loaded assets
				TArray<UObject*> LoadedAssets;
				Handle->GetLoadedAssets(LoadedAssets);

				FDanzmannAssetManagerBundle& BundleToStoreLoadedAssets = Bundles.FindOrAdd(Bundle);

				for (const UObject* LoadedAsset : LoadedAssets)
				{
					BundleToStoreLoadedAssets.AddAsset(LoadedAsset);
				}

				// Execute caller's delegate
				CallerDelegate.Execute(Handle);
			}
		)
	);

	return StreamableHandle;
}

void UDanzmannAssetManager::UnloadLoadedSoftAssets(const TArray<TSoftObjectPtr<UObject>>& AssetsToUnload)
{
	TArray<EDanzmannAssetBundle> Keys;
	Bundles.GetKeys(Keys);

	for (const EDanzmannAssetBundle Key : Keys)
	{
		FDanzmannAssetManagerBundle* Bundle = Bundles.Find(Key);

		for (const TSoftObjectPtr<UObject>& AssetToUnload : AssetsToUnload)
		{
			Bundle->RemoveAsset(AssetToUnload);
		}
	}
}

void UDanzmannAssetManager::UnloadLoadedSoftAssets(const TArray<UObject*>& AssetsToUnload)
{
	TArray<EDanzmannAssetBundle> Keys;
	Bundles.GetKeys(Keys);

	for (const EDanzmannAssetBundle Key : Keys)
	{
		FDanzmannAssetManagerBundle* Bundle = Bundles.Find(Key);

		for (const UObject* AssetToUnload : AssetsToUnload)
		{
			Bundle->RemoveAsset(AssetToUnload);
		}
	}
}

void UDanzmannAssetManager::UnloadLoadedSoftAssets(const EDanzmannAssetBundle BundleToUnload)
{
	if (Bundles.Contains(BundleToUnload))
	{
		Bundles.Remove(BundleToUnload);
	}
}

void UDanzmannAssetManager::DumpLoadedSoftAssets()
{
	checkf(UDanzmannAssetManager::IsInitialized(), TEXT("[%hs] UDanzmannAssetManager isn't initialized."), __FUNCTION__);

	UDanzmannAssetManager& AssetManager = Get();

	UE_LOG(LogDanzmannAssetNexus, Log, TEXT("=========== Danzmann Asset Manager Loaded Soft Assets ==========="));

	for (TPair<EDanzmannAssetBundle, FDanzmannAssetManagerBundle>& Bundle : AssetManager.Bundles)
	{
		UE_LOG(LogDanzmannAssetNexus, Log, TEXT("\tBundle: %s"), *UEnum::GetValueAsString(Bundle.Key));
		
		for (const UObject* Asset : Bundle.Value.GetAssets())
		{
			UE_LOG(LogDanzmannAssetNexus, Log, TEXT("\t  -> Asset: %s"), *Asset->GetName());
		}
	}
}

/*
 * CVars
 */

static FAutoConsoleCommand CVarDumpLoadedSoftAssets(
	TEXT("DanzmannAssetManager.DumpLoadedSoftAssets"),
	TEXT("Shows a list of currently Loaded soft assets by the Danzmann Asset Manager."),
	FConsoleCommandDelegate::CreateStatic(UDanzmannAssetManager::DumpLoadedSoftAssets),
	ECVF_Cheat
);
