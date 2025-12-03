#include "NsUILayer.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/PanelWidget.h"

UNsUILayer::UNsUILayer(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , RootPanel(nullptr)
    , Layer(ENsUILayer::Game)
{
    bIsFocusable = false;
}

TSharedRef<SWidget> UNsUILayer::RebuildWidget()
{
    if (WidgetTree)
    {
        UOverlay* OverlayRoot = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
        WidgetTree->RootWidget = OverlayRoot;
        RootPanel = OverlayRoot;
    }

    return Super::RebuildWidget();
}

void UNsUILayer::AddScreen(UUserWidget* Screen)
{
    if (RootPanel && Screen)
    {
        RootPanel->AddChild(Screen);
    }
}

void UNsUILayer::RemoveScreen(UUserWidget* Screen)
{
    if (RootPanel && Screen)
    {
        RootPanel->RemoveChild(Screen);
    }
}

void UNsUILayer::ClearScreens()
{
    if (RootPanel)
    {
        RootPanel->ClearChildren();
    }
}

void UNsUILayer::SetLayerType(ENsUILayer InLayer)
{
    Layer = InLayer;
}
