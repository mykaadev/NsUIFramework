// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUILayout.h"
#include "Components/Overlay.h"
#include "NsUILayer.h"
#include "NsUILibrary.h"

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
    if ( UNsUILayer* const FoundLayer = *RegisteredLayers.Find(InLayerType))
    {
        FoundLayer->RemoveFromParent();
    }

    return RegisteredLayers.Remove(InLayerType) > 0;
}

UNsUIWidgetStackElement* UNsUILayout::PushContentToLayer(const ENsUILayerType InLayerType, const TSoftClassPtr<class UUserWidget>& InWidgetClass)
{
    if (UNsUILayer* const FoundLayer = *RegisteredLayers.Find(InLayerType))
    {
        if (UNsUIWidgetStackElement* Content = FoundLayer->Push(InWidgetClass))
        {
            ForceLayoutPrepass();
            // The new content that was just pushed
            Content->WidgetScopeLayerType = InLayerType; // fill the layer where he belongs to
            return Content;
        }
    }

    
    return nullptr;
}

UNsUIWidgetStackElement* UNsUILayout::PopContentFromLayer(const ENsUILayerType InLayerType)
{
    if (UNsUILayer* const FoundLayer = *RegisteredLayers.Find(InLayerType))
    {
        // if the layer stack is not empty, we can just keep swapping from widget to widget within that stack
        // if the layer stack is empty, we use the invoker info, now we have to return the focus to the original widget that pushed this one

        UNsUIWidgetStackElement* TopElementOrInvokerInfo = FoundLayer->Pop();
        ForceLayoutPrepass();
        return FindInvokerWidgetInLayout(TopElementOrInvokerInfo);
    }

    return nullptr;
}

UNsUIWidgetStackElement* UNsUILayout::RemoveWidgetFromLayer(const ENsUILayerType InLayerType, UUserWidget* InWidget)
{
    if (UNsUILayer* const FoundLayer = *RegisteredLayers.Find(InLayerType))
    {
        UNsUIWidgetStackElement* TopElementOrInvokerInfo = FoundLayer->RemoveWidgetFromStack(InWidget);
        ForceLayoutPrepass();
        return FindInvokerWidgetInLayout(TopElementOrInvokerInfo);
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
    if (UNsUILayer* const FoundLayer = *RegisteredLayers.Find(InLayerType))
    {
        FoundLayer->ClearStack();
    }
}

UNsUIWidgetStackElement* UNsUILayout::FindInvokerWidgetInLayout(const UNsUIWidgetStackElement* InWidget)
{
    if (InWidget != nullptr)
    {
        // if this widget was pushed from other widget within or the same layer or not
        if (InWidget->InvokerWidgetScope != nullptr && InWidget->InvokerElement != nullptr)
        {
            if (UNsUILayer* const FoundLayer = *RegisteredLayers.Find(InWidget->InvokerLayerType))
            {
                return FoundLayer->FindStackElement(InWidget->InvokerWidgetScope, InWidget->InvokerElement);
            }     
        } // if the widget does not have an invoker (could be the first widget summoned to the layer) we retrieve it again
        else if (InWidget->WidgetScope != nullptr && InWidget->WidgetScopeFocusedElement != nullptr)
        {
            if (UNsUILayer* const FoundLayer = *RegisteredLayers.Find(InWidget->WidgetScopeLayerType))
            {
                return FoundLayer->FindStackElement(InWidget->WidgetScope, InWidget->WidgetScopeFocusedElement);
            }     
        }
       
    }

    return nullptr;
}
