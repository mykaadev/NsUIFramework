// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUILayout.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "NsUILayer.h"

void UNsUILayout::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    EnsureDefaultLayers();
}

void UNsUILayout::RegisterLayer(FName LayerName, UNsUILayer* Layer)
{
    if (!LayerName.IsNone() && Layer != nullptr)
    {
        RegisteredLayers.FindOrAdd(LayerName) = Layer;
    }
}

bool UNsUILayout::UnregisterLayer(FName LayerName)
{
    if (UNsUILayer** Found = RegisteredLayers.Find(LayerName))
    {
        if (UNsUILayer* Layer = *Found)
        {
            Layer->RemoveFromParent();
        }
        RegisteredLayers.Remove(LayerName);
        return true;
    }
    return false;
}

UNsUILayer* UNsUILayout::GetLayer(FName LayerName) const
{
    if (const TObjectPtr<UNsUILayer>* Found = RegisteredLayers.Find(LayerName))
    {
        return Found->Get();
    }
    return nullptr;
}

void UNsUILayout::ClearAllLayers() const
{
    for (const auto& Pair : RegisteredLayers)
    {
        if (UNsUILayer* Layer = Pair.Value)
        {
            Layer->PopToRoot();
        }
    }
}

void UNsUILayout::EnsureDefaultLayers()
{
    if (WidgetTree == nullptr)
    {
        WidgetTree = NewObject<UWidgetTree>(this, UWidgetTree::StaticClass(), TEXT("WidgetTree"));
    }

    UOverlay* RootOverlay = Cast<UOverlay>(WidgetTree->RootWidget);
    if (RootOverlay == nullptr)
    {
        RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
        WidgetTree->RootWidget = RootOverlay;
    }

    auto EnsureLayer = [&](TObjectPtr<UNsUILayer>& Slot, const FName Name)
    {
        if (Slot == nullptr)
        {
            if (UNsUILayer* NewLayer = WidgetTree->ConstructWidget<UNsUILayer>(UNsUILayer::StaticClass(), Name))
            {
                if (UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(NewLayer))
                {
                    OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
                    OverlaySlot->SetVerticalAlignment(VAlign_Fill);
                }
                Slot = NewLayer;
            }
        }

        if (Slot)
        {
            RegisterLayer(Name, Slot);
        }
    };

    EnsureLayer(GameStaticLayer, TEXT("GameStatic"));
    EnsureLayer(GameMenuLayer, TEXT("GameMenu"));
    EnsureLayer(GameModalLayer, TEXT("GameModal"));
    EnsureLayer(ApplicationStaticLayer, TEXT("ApplicationStatic"));
    EnsureLayer(ApplicationMenuLayer, TEXT("ApplicationMenu"));
    EnsureLayer(ApplicationModalLayer, TEXT("ApplicationModal"));
}
