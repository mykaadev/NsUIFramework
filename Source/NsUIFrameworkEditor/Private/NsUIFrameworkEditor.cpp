#include "NsUIFrameworkEditor.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "NsUISubsystem.h"
#include "Smoke/NsUISmokeTestWidgets.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
UNsUISubsystem* GetHost()
{
    const UGameInstance* Game = GEngine && GEngine->GameViewport ? GEngine->GameViewport->GetGameInstance() : nullptr;
    const ULocalPlayer* Player = Game ? Game->GetFirstGamePlayer() : nullptr;
    return Player ? Player->GetSubsystem<UNsUISubsystem>() : nullptr;
}
const FName InspectorTab(TEXT("NsUIInspector"));
TWeakObjectPtr<UUserWidget> Smoke;
}
class FNsUIInspector
{
public:
    static FText Read()
    {
        const UNsUISubsystem* Host = GetHost();
        return FText::FromString(Host ? Host->DescribeState() : TEXT("No local player."));
    }
};
void FNsUIFrameworkEditorModule::StartupModule()
{
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(InspectorTab, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&)
    {
        return SNew(SDockTab).TabRole(ETabRole::NomadTab)
            [SNew(SScrollBox) + SScrollBox::Slot()[SNew(STextBlock).Text_Static(&FNsUIInspector::Read)]];
    })).SetDisplayName(FText::FromString(TEXT("NsUI Inspector")));
    InspectorCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("ns.UI.Inspect"), TEXT("Open the NsUI inspector."),
        FConsoleCommandDelegate::CreateLambda([] { FGlobalTabmanager::Get()->TryInvokeTab(InspectorTab); }), ECVF_Default);
    SmokeTestToggleCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("ns.UI.SmokeTest.Toggle"),
        TEXT("Open or close the NsUI smoke screen."), FConsoleCommandDelegate::CreateLambda([]
        {
            if (UNsUISubsystem* Host = GetHost())
            {
                if (Smoke.IsValid() && Smoke->GetParent())
                {
                    Host->CloseScreen(Smoke.Get());
                    Smoke.Reset();
                }
                else { Smoke = Host->OpenScreen(UNsUISmokeTestRootWidget::StaticClass(), nullptr, ENsUILayer::Menu); }
            }
        }), ECVF_Default);
}
void FNsUIFrameworkEditorModule::ShutdownModule()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(InspectorTab);
    if (InspectorCommand) { IConsoleManager::Get().UnregisterConsoleObject(InspectorCommand); }
    if (SmokeTestToggleCommand) { IConsoleManager::Get().UnregisterConsoleObject(SmokeTestToggleCommand); }
    Smoke.Reset();
}
IMPLEMENT_MODULE(FNsUIFrameworkEditorModule, NsUIFrameworkEditor)
