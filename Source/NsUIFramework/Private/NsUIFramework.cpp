#include "NsUIFramework.h"

#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Modules/ModuleManager.h"
#include "NsUISubsystem.h"

FNsUIFrameworkModule::FNsUIFrameworkModule()
    : InspectCommand(nullptr)
{
}

void FNsUIFrameworkModule::StartupModule()
{
#if !UE_BUILD_SHIPPING
    InspectCommand = IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("ns.UI.Inspect"),
        TEXT("Logs the open UI widgets, the active widget, and the input state of each local player"),
        FConsoleCommandWithWorldDelegate::CreateStatic(&FNsUIFrameworkModule::Inspect),
        ECVF_Default);
#endif
}

void FNsUIFrameworkModule::ShutdownModule()
{
    if (InspectCommand != nullptr)
    {
        IConsoleManager::Get().UnregisterConsoleObject(InspectCommand);
        InspectCommand = nullptr;
    }
}

void FNsUIFrameworkModule::Inspect(UWorld* const World)
{
    const UGameInstance* const GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
    if (GameInstance == nullptr)
    {
        UE_LOG(LogNsUI, Display, TEXT("ns.UI.Inspect needs a game world"));
        return;
    }
    for (const ULocalPlayer* const Player : GameInstance->GetLocalPlayers())
    {
        if (const UNsUISubsystem* const UI = Player != nullptr ? Player->GetSubsystem<UNsUISubsystem>() : nullptr)
        {
            UE_LOG(LogNsUI, Display, TEXT("%s"), *UI->DescribeState());
        }
    }
}

IMPLEMENT_MODULE(FNsUIFrameworkModule, NsUIFramework)
