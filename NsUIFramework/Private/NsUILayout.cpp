// Copyright (C) 2024 mykaa. All rights reserved.

#include "NsUILayout.h"
#include "Components/Overlay.h"
#include "NsUILayer.h"

void UNsUILayout::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    RegisterLayer(ENsUILayerType::GameStatic, GameStaticLayer);
    RegisterLayer(ENsUILayerType::GameMenu, GameMenuLayer);
    RegisterLayer(ENsUILayerType::GameModal, GameModalLayer);
    RegisterLayer(ENsUILayerType::ApplicationStatic, ApplicationStaticLayer);
    RegisterLayer(ENsUILayerType::ApplicationMenu, ApplicationMenuLayer);
    RegisterLayer(ENsUILayerType::ApplicationModal, ApplicationModalLayer);
}

void UNsUILayout::RegisterLayer(const ENsUILayerType InLayerType, class UNsUILayer* const InLayer)
{
    if (InLayer != nullptr && !RegisteredLayers.Contains(InLayerType))
    {
        RegisteredLayers.Emplace(InLayerType, InLayer);
    }
}

bool UNsUILayout::UnregisterLayer(const ENsUILayerType InLayerType)
{
    UNsUILayer* const FoundLayer = *RegisteredLayers.Find(InLayerType);
    if (FoundLayer != nullptr)
    {
        FoundLayer->RemoveFromParent();
    }

    return RegisteredLayers.Remove(InLayerType) > 0;
}

UUserWidget* UNsUILayout::PushContentToLayer(const ENsUILayerType InLayerType, const TSoftClassPtr<class UUserWidget>& InWidgetClass)
{
    UNsUILayer* const FoundLayer = *RegisteredLayers.Find(InLayerType);
    if (FoundLayer != nullptr)
    {
        return FoundLayer->Push(InWidgetClass);
    }

    return nullptr;
}

UUserWidget* UNsUILayout::PopContentFromLayer(const ENsUILayerType InLayerType)
{
    UNsUILayer* const FoundLayer = *RegisteredLayers.Find(InLayerType);
    if (FoundLayer != nullptr)
    {
        return FoundLayer->Pop();
    }

    return nullptr;
}

void UNsUILayout::ClearAllLayers() const
{
    TArray<UNsUILayer*> InOutLayers; RegisteredLayers.GenerateValueArray(InOutLayers);
    for (UNsUILayer* const Layer : InOutLayers)
    {
        if (Layer != nullptr)
        {
            Layer->ClearStack();
        }
    }
}

void UNsUILayout::ClearLayer(const ENsUILayerType InLayerType)
{
    UNsUILayer* const FoundLayer = *RegisteredLayers.Find(InLayerType);
    if (FoundLayer != nullptr)
    {
        FoundLayer->ClearStack();
    }
}