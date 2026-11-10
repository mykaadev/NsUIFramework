#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"
#include "NsUISubsystem.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

/** The editor module of NsUIFramework. It owns the NsUI Inspector tab */
class FNsUIFrameworkEditorModule : public IModuleInterface
{
// Functions
public:
    //~ Begin IModuleInterface
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    //~ End IModuleInterface

private:
    /** Makes the inspector tab */
    static TSharedRef<SDockTab> SpawnInspector(const FSpawnTabArgs& Args);

    /** Returns the state of the UI of every local player in the play worlds */
    static FText DescribePlayWorlds();

    /** Returns the name of the inspector tab */
    static FName GetInspectorTabName() { return TEXT("NsUIInspector"); }
};

void FNsUIFrameworkEditorModule::StartupModule()
{
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GetInspectorTabName(), FOnSpawnTab::CreateStatic(&FNsUIFrameworkEditorModule::SpawnInspector))
        .SetDisplayName(NSLOCTEXT("NsUIFrameworkEditor", "InspectorTitle", "NsUI Inspector"))
        .SetTooltipText(NSLOCTEXT("NsUIFrameworkEditor", "InspectorTooltip", "Shows the open UI widgets and the input state of each local player in play"));
}

void FNsUIFrameworkEditorModule::ShutdownModule()
{
    if (FSlateApplication::IsInitialized())
    {
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GetInspectorTabName());
    }
}

TSharedRef<SDockTab> FNsUIFrameworkEditorModule::SpawnInspector(const FSpawnTabArgs& Args)
{
    TSharedRef<STextBlock> Text = SNew(STextBlock).Text(DescribePlayWorlds());
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SButton)
                .Text(NSLOCTEXT("NsUIFrameworkEditor", "Refresh", "Refresh"))
                .OnClicked_Lambda([Text]()
                {
                    Text->SetText(DescribePlayWorlds());
                    return FReply::Handled();
                })
            ]
            + SVerticalBox::Slot().FillHeight(1.f)
            [
                SNew(SScrollBox) + SScrollBox::Slot()[Text]
            ]
        ];
}

FText FNsUIFrameworkEditorModule::DescribePlayWorlds()
{
    FString Result;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        const UWorld* const World = Context.World();
        const UGameInstance* const GameInstance = World != nullptr && World->IsGameWorld() ? World->GetGameInstance() : nullptr;
        if (GameInstance == nullptr)
        {
            continue;
        }
        for (const ULocalPlayer* const Player : GameInstance->GetLocalPlayers())
        {
            if (const UNsUISubsystem* const UI = Player != nullptr ? Player->GetSubsystem<UNsUISubsystem>() : nullptr)
            {
                Result += FString::Printf(TEXT("%s\n%s\n\n"), *World->GetName(), *UI->DescribeState());
            }
        }
    }
    return FText::FromString(Result.IsEmpty() ? TEXT("No game world with an NsUI subsystem") : Result);
}

IMPLEMENT_MODULE(FNsUIFrameworkEditorModule, NsUIFrameworkEditor)
