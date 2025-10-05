# Plugin Name
An extension of Asset Manager that allows similar behavior
of Loading, Preloading and Unloading Primary Assets, but for
any `TSoftObjectPtr` or `TSoftClassPtr` as well.

Dependencies:
* [DanzmannMetaprogrammingUtils](https://github.com/iVcente/DanzmannMetaprogrammingUtils).

## Usage Example

#### Load TSoftObjectPtr<> and TSoftClassPtr<>
Loaded assets stay in memory until explicitly told otherwise.
```cpp
// Callback to be triggered when assets are Loaded.
FStreamableDelegateWithHandle Delegate = FStreamableDelegateWithHandle::CreateLambda(
    [this]
    (const TSharedPtr<FStreamableHandle>& StreamableHandle)
    {
        // You can access the loaded assets through the FStreamableHandle
        TArray<UObject*> Objects;
        StreamableHandle->GetLoadedAssets(Objects);

        for (const UObject* Object : Objects)
        {
            UE_LOG(LogTemp, Warning, TEXT("Loaded Object: %s"), *Object->GetName());
        }
    }
);

TArray<TSoftObjectPtr<UObject>> ObjectsToLoad;
const TArray<TSoftClassPtr<APawn>> ClassesToLoad;

// You can specify a Bundle -- in the form a FGameplayTag -- to organize the Loaded assets.
// If no Bundle is passed, all assets are stored in a default Bundle.
UDanzmannAssetManager::Get()->LoadSoftAssets(ObjectsToLoad, MoveTemp(Delegate), FGameplayTag());
TSharedPtr<FStreamableHandle> StreamableHandle = UDanzmannAssetManager::Get()->LoadSoftAssets(ClassesToLoad, MoveTemp(Delegate));

...

// Once the assets are Loaded, besides the callback, you can also access them as the following:
ObjectsToLoad[0]->Get(); // Valid!

TArray<UObject*> Objects;
StreamableHandle->GetLoadedAssets(Objects); // Valid!
```

---

### Preload TSoftObjectPtr<> and TSoftClassPtr<>
Unlike Loaded assets, Preloaded assets require a strong reference to keep them alive. So you either need to keep the `FStreamableHandle` or store a reference to the assets once the callback is triggerd.
```cpp
// Callback to be triggered when assets are Preloaded.
FStreamableDelegateWithHandle Delegate = FStreamableDelegateWithHandle::CreateLambda(
    [this]
    (const TSharedPtr<FStreamableHandle>& StreamableHandle)
    {
        // In case you'd like to keep these loaded assets alive, you need to store
        // FStreamableHandle when Preload is called, or keep a strong
        // reference to the assets here -- like for instance, assigning them to
        // TObjectPtr<> member variables.

        TArray<UObject*> Objects;
        StreamableHandle->GetLoadedAssets(Objects);

        for (const UObject* Object : Objects)
        {
            UE_LOG(LogTemp, Warning, TEXT("Preloaded Object: %s"), *Object->GetName());
        }
    }
);

TArray<TSoftObjectPtr<UObject>> ObjectsToPreload;
const TArray<TSoftClassPtr<APawn>> ClassesToPreload;

UDanzmannAssetManager::Get()->PreloadSoftAssets(ObjectsToPreload, MoveTemp(Delegate));
TSharedPtr<FStreamableHandle> StreamableHandle = UDanzmannAssetManager::Get()->PreloadSoftAssets(ClassesToPreload, MoveTemp(Delegate));

...

// Once the assets are Preloaded, besides the callback you can also access them as the following:
ObjectsToLoad[0]->Get(); // Not valid! Unless you stored a reference to assets on callback.

TArray<UObject*> Objects;
StreamableHandle->GetLoadedAssets(Objects); // Valid!
```

---

### Unload Assets
You can unload assets tracked by the Asset Manager in many different ways:
* By `TSoftObjectPtr<>`;
* By `TSoftClassPtr<>`;
* By `UObject*` (or any class derived from `UObject`);
* By Bundle.

```cpp
// Unload by TSoftObjectPtr<>.
TArray<TSoftObjectPtr<UObject>> SoftObjects;
UDanzmannAssetManager::Get()->UnloadLoadedSoftAssets(SoftObjects);

// Unload by TSoftClassPtr<>.
TArray<TSoftClassPtr<UObject>> SoftClasses;
UDanzmannAssetManager::Get()->UnloadLoadedSoftAssets(SoftClasses);

// Unload by UObject*.
TArray<UObject*> Objects;
UDanzmannAssetManager::Get()->UnloadLoadedSoftAssets(Objects);

// Unload by Bundle. Suppose we have the following Bundles:
// * AssetsBundle.UI;
// * AssetsBundle.UI.Level01;
// * AssetsBundle.UI.Level02;
// * AssetBundle.Default.
// Passing Bundle as AssetsBundle.UI and bExatcMatch as true will unload only assets in this Bundle.
// Passing Bundle as AssetsBundle.UI and bExatcMatch as false will unload all assets in this Bundle plus AssetsBundle.UI.Level01 and AssetsBundle.UI.Level02.
FGameplayTag Bundle;
UDanzmannAssetManager::Get()->UnloadLoadedSoftAssets(Bundle, true);
```
