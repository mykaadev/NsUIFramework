#include "NsUIFramework.h"

#include "NsUISlateInputRouter.h"
#include "Misc/CoreDelegates.h"

#define LOCTEXT_NAMESPACE "FNsUIFrameworkModule"

void FNsUIFrameworkModule::StartupModule()
{
    FNsUISlateInputRouter::Startup();
    EngineInitializedHandle = FCoreDelegates::OnPostEngineInit.AddStatic(&FNsUISlateInputRouter::Startup);
}

void FNsUIFrameworkModule::ShutdownModule()
{
    FNsUISlateInputRouter::Shutdown();
    FCoreDelegates::OnPostEngineInit.Remove(EngineInitializedHandle);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNsUIFrameworkModule, NsUIFramework)
