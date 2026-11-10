#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "NsUIFrameworkTypes.h"
#include "UObject/WeakObjectPtr.h"

/** Reads the keys of the UI actions from Enhanced Input for one local player, and owns the Slate navigation config of that player while a widget takes input */
class FNsUIInputActions
{
// Functions
public:
    /** Constructor */
    explicit FNsUIInputActions(class ULocalPlayer* InPlayer);

    /** Gives back the earlier navigation config */
    ~FNsUIInputActions();

    /** Adds the UI mapping context to the player and reads the keys again */
    void AddMappingContext();

    /** Reads the keys of each UI action from the player mappings. Call it when Enhanced Input rebuilds its mappings */
    void ReadKeys();

    /** Returns the UI action of a key, or an unset value */
    TOptional<ENsUIAction> FindAction(const FKey& Key) const;

    /** Returns the keys of a UI action */
    TArray<FKey> GetKeys(ENsUIAction Action) const;

    /** Returns the UI keys as text, for debug */
    FString DescribeKeys() const;

    /** Installs the navigation config of the UI keys. Analog true lets the sticks move the focus */
    void InstallNavigation(bool bAnalog);

    /** Gives back the navigation config that was there before InstallNavigation */
    void RemoveNavigation();

private:
    /** Builds the navigation config from the keys */
    void BuildNavigation();

// Variables
private:
    /** The local player of the keys */
    TWeakObjectPtr<class ULocalPlayer> Player;

    /** The UI action of each key */
    TMap<FKey, ENsUIAction> ActionOfKey;

    /** The navigation config of the UI keys */
    TSharedPtr<class FNavigationConfig> Navigation;

    /** The navigation config of the player before InstallNavigation */
    TSharedPtr<class FNavigationConfig> PreviousNavigation;

    /** True while the UI navigation config is installed */
    bool bInstalled;

    /** True after the missing settings were logged one time */
    bool bReportedMissingSettings;
};
