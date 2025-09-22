// Copyright (C) 2025 Dancing Man Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DanzmannAssetBundle.h"
#include "Engine/AssetManager.h"

#include "DanzmannAssetManager.generated.h"

/**
 * Struct to hold strong references to objects requested to
 * Load via UDanzmannAssetManager for a given bundle.
 */
USTRUCT()
struct FDanzmannAssetManagerBundle
{
	GENERATED_BODY()

	public:
		const TSet<TObjectPtr<const UObject>>& GetAssets() const
		{
			return LoadedAssets;
		}
	
		/**
		 * Add a given asset reference to this bundle.
		 * @param Asset Asset to add to bundle.
		 */
		void AddAsset(const UObject* Asset)
		{
			LoadedAssets.Add(Asset);
		}

		/**
		 * Remove a given asset reference from this bundle.
		 * @param Asset Asset to remove from bundle.
		 */
		void RemoveAsset(const UObject* Asset)
		{
			LoadedAssets.Remove(Asset);
		}

		/**
		 * Remove a given asset reference from this bundle.
		 * @param Asset Asset to remove from bundle.
		 */
		void RemoveAsset(const TSoftObjectPtr<UObject>& Asset)
		{
			LoadedAssets.Remove(Asset.Get());
		}

		/**
		 * Clear all references to assets that this bundle holds.
		 */
		void Clear()
		{
			LoadedAssets.Empty();
		}
	
	private:
		/**
		 * Loaded assets that are part of this bundle.
		 */
		UPROPERTY()
		TSet<TObjectPtr<const UObject>> LoadedAssets;
};

/**
 * An extension of Asset Manager that allows similar behavior
 * of Preloading, Loading and Unloading Primary Assets, but for
 * any TSoftObjectPtr or TSoftClassPtr as well.
 */
UCLASS()
class DANZMANNASSETNEXUS_API UDanzmannAssetManager : public UAssetManager
{
	GENERATED_BODY()

	public:
		/**
		 * Get a reference to Danzmann Asset Manager.
		 * @return A reference to UDanzmannAssetManager.
		 */
		static UDanzmannAssetManager& Get();

		static void DumpLoadedSoftAssets();
	
		/**
		 * Preload a list of assets (as TSoftObjectPtr) asynchronously. You must either
		 * keep the returned handle or store a strong reference to the Preloaded assets
		 * somewhere else, otherwise the assets will be garbage collected. 
		 * @param AssetsToPreload Assets to Preload.
		 * @param Delegate Delegate to trigger callback when assets have been Preloaded.
		 * @note If the assets are already in memory, Delegate will broadcast right away. 
		 * @return Handle with Preloaded assets.
		 */
		TSharedPtr<FStreamableHandle> PreloadSoftAssets(const TArray<TSoftObjectPtr<UObject>>& AssetsToPreload, FStreamableDelegateWithHandle&& Delegate);

		/**
		 * Load a list of assets (as TSoftObjectPtr) asynchronously. Once Loaded, these
		 * assets will remain in memory until explicitly unloaded -- and there are no
		 * strong references to them. 
		 * @param AssetsToLoad Assets to Load.
		 * @param Delegate Delegate to trigger callback when assets have been Loaded.
		 * @param Bundle Bundle to store references to the assets.
		 * @note If the assets are already in memory, Delegate will broadcast right away. 
		 * @return Handle with Loaded assets.
		 */
		TSharedPtr<FStreamableHandle> LoadSoftAssets(const TArray<TSoftObjectPtr<UObject>>& AssetsToLoad, FStreamableDelegateWithHandle&& Delegate, const EDanzmannAssetBundle Bundle = EDanzmannAssetBundle::Default);

		/**
		 * Unload a list of soft assets previously Loaded by the Asset Manager.
		 * @param AssetsToUnload Assets to unload.
		 */
		void UnloadLoadedSoftAssets(const TArray<TSoftObjectPtr<UObject>>& AssetsToUnload);

		/**
		 * Unload a list of soft assets previously Loaded by the Asset Manager.
		 * @param AssetsToUnload Assets to unload.
		 */
		void UnloadLoadedSoftAssets(const TArray<UObject*>& AssetsToUnload);

		/**
		 * Unload all assets within a given bundle. 
		 * @param BundleToUnload Bundle to unload all assets within.
		 */
		void UnloadLoadedSoftAssets(const EDanzmannAssetBundle BundleToUnload);

	private:
		/**
		 * Map that holds strong references to Loaded assets within bundles.
		 * Its purpose is to allow Loaded assets to remain in memory until
		 * explicitly told otherwise.
		 */
		UPROPERTY()
		TMap<EDanzmannAssetBundle, FDanzmannAssetManagerBundle> Bundles;
};
