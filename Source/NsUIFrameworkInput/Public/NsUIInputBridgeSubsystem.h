#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "NsUIInputBridgeSubsystem.generated.h"

/** Tells NsInput which device the player uses in the UI, so the prompt icons switch in menus too. Does nothing without NsInput */
UCLASS()
class NSUIFRAMEWORKINPUT_API UNsUIInputBridgeSubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

// Functions
public:
    //~ Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface

private:
    /** Gives a device change of the UI to NsInput */
    void HandleDeviceUsed(bool bGamepad);

// Variables
private:
    /** The binding of the device event of the NsUI subsystem */
    FDelegateHandle DeviceHandle;
};
