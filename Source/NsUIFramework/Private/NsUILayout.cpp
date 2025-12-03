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

void UNsUILayout::RegisterLayer(ENsUILayer LayerSlot, UNsUILayer* Layer)
{
    if (Layer != nullptr)
    {
        RegisteredLayers.FindOrAdd(LayerSlot) = Layer;
    }
}

bool UNsUILayout::UnregisterLayer(ENsUILayer LayerSlot)
{
    if (UNsUILayer** Found = RegisteredLayers.Find(LayerSlot))
    {
        if (UNsUILayer* Layer = *Found)
        {
            Layer->RemoveFromParent();
        }
        RegisteredLayers.Remove(LayerSlot);
        return true;
    }
    return false;
}

UNsUILayer* UNsUILayout::GetLayer(ENsUILayer LayerSlot) const
{
    if (const TObjectPtr<UNsUILayer>* Found = RegisteredLayers.Find(LayerSlot))
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

    auto EnsureLayer = [&](TObjectPtr<UNsUILayer>& Slot, const FName Name, ENsUILayer LayerKey)
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
            RegisterLayer(LayerKey, Slot);
        }
    };

    EnsureLayer(MenuLayer, TEXT("Menu"), ENsUILayer::Menu);
    EnsureLayer(ModalLayer, TEXT("Modal"), ENsUILayer::Modal);
}
