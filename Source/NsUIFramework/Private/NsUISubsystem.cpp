// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "NsUILayout.h"
#include "NsUILayer.h"

UNsUISubsystem::UNsUISubsystem()
    : ActiveUILayout(nullptr)
{
}

void UNsUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UNsUISubsystem::Deinitialize()
{
    ActiveUILayout = nullptr;
    Super::Deinitialize();
}

UNsUISubsystem* UNsUISubsystem::GetSubsystem()
{
    if (const UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UNsUISubsystem>();
        }
    }
    return nullptr;
}

UUserWidget* UNsUISubsystem::PushWidgetToLayer(const TSoftClassPtr<UUserWidget>& InWidget, ENsUILayerType InLayerType)
{
    if (!ActiveUILayout)
    {
        return nullptr;
    }
    if (UNsUIWidgetStackElement* StackElement = ActiveUILayout->PushContentToLayer(InLayerType, InWidget))
    {
        return StackElement->Widget.Get();
    }
    return nullptr;
}

UUserWidget* UNsUISubsystem::ReplaceTopWidgetOnLayer(const TSoftClassPtr<UUserWidget>& InWidget, ENsUILayerType InLayerType)
{
    if (!ActiveUILayout)
    {
        return nullptr;
    }
    if (UNsUIWidgetStackElement* StackElement = ActiveUILayout->ReplaceTopWidgetOnLayer(InLayerType, InWidget))
    {
        return StackElement->Widget.Get();
    }
    return nullptr;
}

void UNsUISubsystem::PopWidgetFromLayer(ENsUILayerType InLayerType)
{
    if (ActiveUILayout)
    {
        ActiveUILayout->PopContentFromLayer(InLayerType);
    }
}

void UNsUISubsystem::RemoveWidgetFromLayer(ENsUILayerType InLayerType, UUserWidget* InWidget)
{
    if (ActiveUILayout)
    {
        ActiveUILayout->RemoveWidgetFromLayer(InLayerType, InWidget);
    }
}

void UNsUISubsystem::ClearLayer(ENsUILayerType InLayerType)
{
    if (ActiveUILayout)
    {
        ActiveUILayout->ClearLayer(InLayerType);
    }
}

UNsUIWidgetStackElement* UNsUISubsystem::ApplyLayerTransition(const FNsUILayerTransitionRequest& Request)
{
    switch (Request.Operation)
    {
    case ENsUILayerTransitionOp::Push:
        return ActiveUILayout ? ActiveUILayout->PushContentToLayer(Request.LayerType, Request.WidgetClass) : nullptr;
    case ENsUILayerTransitionOp::ReplaceTop:
        return ActiveUILayout ? ActiveUILayout->ReplaceTopWidgetOnLayer(Request.LayerType, Request.WidgetClass) : nullptr;
    case ENsUILayerTransitionOp::Pop:
        return ActiveUILayout ? ActiveUILayout->PopContentFromLayer(Request.LayerType) : nullptr;
    case ENsUILayerTransitionOp::Remove:
        return ActiveUILayout ? ActiveUILayout->RemoveWidgetFromLayer(Request.LayerType, Request.WidgetInstance) : nullptr;
    case ENsUILayerTransitionOp::ClearLayer:
        if (ActiveUILayout)
        {
            ActiveUILayout->ClearLayer(Request.LayerType);
        }
        return nullptr;
    default:
        return nullptr;
    }
}

void UNsUISubsystem::SetActiveUILayout(const TSoftClassPtr<UNsUILayout>& InLayout)
{
    if (!GetWorld())
    {
        return;
    }

    UClass* LayoutClass = InLayout.LoadSynchronous();
    if (!LayoutClass)
    {
        return;
    }

    if (ActiveUILayout)
    {
        ActiveUILayout->RemoveFromParent();
        ActiveUILayout = nullptr;
    }

    ActiveUILayout = CreateWidget<UNsUILayout>(GetWorld(), LayoutClass);
    if (ActiveUILayout)
    {
        ActiveUILayout->AddToViewport();
    }
}

UNsUILayout* UNsUISubsystem::GetActiveUILayout() const
{
    return ActiveUILayout;
}
