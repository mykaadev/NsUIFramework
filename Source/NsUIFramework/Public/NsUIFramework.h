#pragma once

#include "Modules/ModuleManager.h"

class FNsUIFrameworkModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
private:
    FDelegateHandle EngineInitializedHandle;
};
