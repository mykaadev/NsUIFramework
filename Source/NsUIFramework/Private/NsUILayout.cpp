// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUILayout.h"

#include "NsUILayer.h"

void UNsUILayout::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (GameStaticLayer)
    {
        RegisterLayer(ENsUILayerType::GameStatic, GameStaticLayer);
    }
    if (GameMenuLayer)
    {
        RegisterLayer(ENsUILayerType::GameMenu, GameMenuLayer);
    }
    if (GameModalLayer)
    {
        RegisterLayer(ENsUILayerType::GameModal, GameModalLayer);
    }
    if (ApplicationStaticLayer)
    {
        RegisterLayer(ENsUILayerType::ApplicationStatic, ApplicationStaticLayer);
    }
    if (ApplicationMenuLayer)
    {
        RegisterLayer(ENsUILayerType::ApplicationMenu, ApplicationMenuLayer);
    }
    if (ApplicationModalLayer)
    {
        RegisterLayer(ENsUILayerType::ApplicationModal, ApplicationModalLayer);
    }
}

void UNsUILayout::RegisterLayer(ENsUILayerType InLayerType, UNsUILayer* InLayer)
{
    if (!InLayer)
    {
        return;
    }
    RegisteredLayers.Add(InLayerType, InLayer);
}

bool UNsUILayout::UnregisterLayer(ENsUILayerType InLayerType)
{
    return RegisteredLayers.Remove(InLayerType) > 0;
}

UNsUIWidgetStackElement* UNsUILayout::PushContentToLayer(ENsUILayerType InLayerType, const TSoftClassPtr<UUserWidget>& InWidgetClass)
{
    if (UNsUILayer** FoundLayer = RegisteredLayers.Find(InLayerType))
    {
        return (*FoundLayer)->Push(InWidgetClass);
    }
    return nullptr;
}

UNsUIWidgetStackElement* UNsUILayout::ReplaceTopWidgetOnLayer(ENsUILayerType InLayerType, const TSoftClassPtr<UUserWidget>& InWidgetClass)
{
    if (UNsUILayer** FoundLayer = RegisteredLayers.Find(InLayerType))
    {
        return (*FoundLayer)->ReplaceTop(InWidgetClass);
    }
    return nullptr;
}

UNsUIWidgetStackElement* UNsUILayout::PopContentFromLayer(ENsUILayerType InLayerType)
{
    if (UNsUILayer** FoundLayer = RegisteredLayers.Find(InLayerType))
    {
        return (*FoundLayer)->Pop();
    }
    return nullptr;
}

UNsUIWidgetStackElement* UNsUILayout::RemoveWidgetFromLayer(ENsUILayerType InLayerType, UUserWidget* InWidget)
{
    if (UNsUILayer** FoundLayer = RegisteredLayers.Find(InLayerType))
    {
        return (*FoundLayer)->RemoveWidgetFromStack(InWidget);
    }
    return nullptr;
}

void UNsUILayout::ClearLayer(ENsUILayerType InLayerType)
{
    if (UNsUILayer** FoundLayer = RegisteredLayers.Find(InLayerType))
    {
        (*FoundLayer)->ClearStack();
    }
}

void UNsUILayout::ClearAllLayers() const
{
    for (const TPair<ENsUILayerType, UNsUILayer*>& Pair : RegisteredLayers)
    {
        if (Pair.Value)
        {
            Pair.Value->ClearStack();
        }
    }
}
