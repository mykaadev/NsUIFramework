#pragma once

#include "Modules/ModuleManager.h"

class IConsoleObject;

class FNsUIFrameworkEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    IConsoleObject* InspectorCommand = nullptr;
    IConsoleObject* SmokeTestToggleCommand = nullptr;
};
