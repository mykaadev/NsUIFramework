// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FNsUIFrameworkModule : public IModuleInterface
{
public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
