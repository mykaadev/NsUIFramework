#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/** The runtime module of NsUIFramework. It owns the debug console command */
class FNsUIFrameworkModule : public IModuleInterface
{
// Functions
public:
    /** Constructor */
    FNsUIFrameworkModule();

    //~ Begin IModuleInterface
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    //~ End IModuleInterface

private:
    /** Logs the state of the UI of each local player of a world */
    static void Inspect(class UWorld* World);

// Variables
private:
    /** The ns.UI.Inspect console command, or null in a Shipping build */
    class IConsoleObject* InspectCommand;
};
