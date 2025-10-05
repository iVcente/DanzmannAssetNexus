// Copyright (C) 2025 Dancing Man Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DanzmannGameplayTags_AssetsBundle.h"
#include "DanzmannIsRawPtrDerivedFrom.h"
#include "DanzmannIsTSoftClassPtrDerivedFrom.h"
#include "DanzmannIsTSoftObjectPtrDerivedFrom.h"
#include "DanzmannToRawPtr.h"
#include "Engine/AssetManager.h"
#include "GameplayTagContainer.h"

#include "DanzmannAssetManager.generated.h"

/**
 * Struct to hold strong references to objects requested to
 * Load via UDanzmannAssetManager for a given Bundle.
 */
USTRUCT()
struct FDanzmannAssetManagerBundle
{
	GENERATED_BODY()

	public:
		/**
		 * Get the name of the assets contained in this Bundle.
		 * @param Out_AssetsName Name of the assets within Bundle as an out parameter.
		 */
		void GetAssetsName(TArray<FString>& Out_AssetsName) const
		{
			Out_AssetsName.Reserve(LoadedAssets.Num());
			
			for (const UObject* LoadedAsset : LoadedAssets)
			{
				Out_AssetsName.Add(LoadedAsset->GetPackage()->GetName());
			}
		}
	
		/**
		 * Add a given asset reference to this Bundle.
		 * @param Asset Asset to add to Bundle.
		 */
		void AddAsset(const UObject* Asset)
		{
			LoadedAssets.Add(Asset);
		}

		/**
		 * Remove a given asset strong reference from this Bundle.
		 * @tparam T UObject*, TSoftObjectPtr<UObject> or TSoftClassPtr<UObject>.
		 * @param Asset Asset to remove from Bundle.
		 * @return The number of elements removed.
		 */
		template<typename T>
		int32 RemoveAsset(const T Asset)
		{
			static_assert(
				Danzmann::IsRawPtrDerivedFrom<T, UObject>() ||
				Danzmann::IsTSoftObjectPtrDerivedFrom<T, UObject>() ||
				Danzmann::IsTSoftClassPtrDerivedFrom<T, UObject>(),
				"Asset must be a raw pointer, TSoftObjectPtr or TSoftClassPtr derived from UObject."
			);
			
			const UObject* RawAsset = Danzmann::ToRawPointer(Asset);
			return LoadedAssets.Remove(RawAsset);
		}

		/**
		 * Clear all references to assets that this Bundle holds.
		 */
		void Clear()
		{
			LoadedAssets.Empty();
		}
	
	private:
		/**
		 * Loaded assets that are part of this Bundle.
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

		/**
		 * Preload a list of assets (as TSoftObjectPtr or TSoftClassPtr) asynchronously.
		 * You must either keep the returned handle or store a strong reference to the
		 * Preloaded assets somewhere else, otherwise the assets will be garbage collected. 
		 * @tparam T TSoftObjectPtr or TSoftClassPtr.
		 * @param AssetsToPreload Assets to Preload.
		 * @param Delegate Delegate to trigger callback when assets have been Preloaded.
		 * @note If the assets are already in memory, Delegate will broadcast right away. 
		 * @return Handle with Preloaded assets.
		 */
		template<typename T>
		TSharedPtr<FStreamableHandle> PreloadSoftAssets(const TArray<T>& AssetsToPreload, FStreamableDelegateWithHandle&& Delegate)
		{
			static_assert(
				Danzmann::IsTSoftObjectPtrDerivedFrom<T, UObject>() ||
				Danzmann::IsTSoftClassPtrDerivedFrom<T, UObject>(),
				"AssetsToPreload must be an array of TSoftObjectPtr or TSoftClassPtr derived from UObject."
			);
				
			TArray<FSoftObjectPath> AssetsPath;
			AssetsPath.Reserve(AssetsToPreload.Num());

			for (const T& AssetToPreload : AssetsToPreload)
			{
				if (!AssetToPreload.IsNull())
				{
					AssetsPath.Add(AssetToPreload.ToSoftObjectPath());
				}
			}
		
			return GetStreamableManager().RequestAsyncLoad(MoveTemp(AssetsPath), MoveTemp(Delegate));
		}

		/**
		 * Load a list of assets (as TSoftObjectPtr or TSoftClassPtr) asynchronously.
		 * Once Loaded, these assets will remain in memory until explicitly
		 * unloaded -- and there are no other strong references to them.
		 * @tparam T TSoftObjectPtr or TSoftClassPtr.
		 * @param AssetsToLoad Assets to Load.
		 * @param Delegate Delegate to trigger callback when assets have been Loaded.
		 * @param Bundle Bundle to store references to the assets.
		 * @note If the assets are already in memory, Delegate will broadcast right away. 
		 * @return Handle with Loaded assets.
		 */
		template<typename T>
		TSharedPtr<FStreamableHandle> LoadSoftAssets(const TArray<T>& AssetsToLoad, FStreamableDelegateWithHandle&& Delegate, const FGameplayTag Bundle = Danzmann::GameplayTags::AssetsBundle_Default)
		{
			static_assert(
				Danzmann::IsTSoftObjectPtrDerivedFrom<T, UObject>() ||
				Danzmann::IsTSoftClassPtrDerivedFrom<T, UObject>(),
				"AssetsToLoad must be an array of TSoftObjectPtr or TSoftClassPtr derived from UObject."
			);
			
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
	
		/**
		 * Unload a list of assets previously Loaded by the Danzmann Asset Manager.
		 * @tparam T Raw pointer, TSoftObjectPtr or TSoftClassPtr derived from UObject.
		 * @param AssetsToUnload Assets to unload.
		 * @return The number of elements removed.
		 */
		template<typename T>
		int32 UnloadLoadedSoftAssets(const TArray<T>& AssetsToUnload)
		{
			static_assert(
				Danzmann::IsRawPtrDerivedFrom<T, UObject>() ||
				Danzmann::IsTSoftObjectPtrDerivedFrom<T, UObject>() ||
				Danzmann::IsTSoftClassPtrDerivedFrom<T, UObject>(),
				"AssetsToUnload must be an array of raw pointer, TSoftObjectPtr or TSoftClassPtr derived from UObject."
			);
			
			TArray<FGameplayTag> Keys;
			Bundles.GetKeys(Keys);

			int32 NumAssetsUnloaded = 0;
			
			for (const FGameplayTag& Key : Keys)
			{
				FDanzmannAssetManagerBundle* Bundle = Bundles.Find(Key);

				for (const T& AssetToUnload : AssetsToUnload)
				{
					NumAssetsUnloaded += Bundle->RemoveAsset(AssetToUnload);
				}
			}

			return NumAssetsUnloaded;
		}
	
		/**
		 * Unload all assets within a given Bundle. 
		 * @param BundleToUnload Bundle to unload all assets within.
		 * @param bExactMatch
		 */
		int32 UnloadLoadedSoftAssets(const FGameplayTag& BundleToUnload, const bool bExactMatch = true);

		/**
		 * Print a list of currently Loaded soft assets by the Danzmann Asset Manager.
		 */
		static void DumpLoadedSoftAssets();

	private:
		/**
		 * Map that holds strong references to Loaded assets within Bundles.
		 * Its purpose is to allow Loaded assets to remain in memory until
		 * explicitly told otherwise.
		 */
		UPROPERTY()
		TMap<FGameplayTag, FDanzmannAssetManagerBundle> Bundles;
};
