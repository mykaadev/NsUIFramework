// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUILibrary.h"

#include "Blueprint/WidgetTree.h"
#include "Engine/Engine.h"
#include "NsUILayout.h"
#include "NsUILayer.h"
#include "NsUISubsystem.h"
#include "NsUIWidget.h"

UUserWidget* UNsUILibrary::AddWidgetToLayer(const TSoftClassPtr<UUserWidget>& InWidget, ENsUILayerType InLayerType)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        return UISubsystem->PushWidgetToLayer(InWidget, InLayerType);
    }
    return nullptr;
}

UUserWidget* UNsUILibrary::ReplaceTopWidgetOnLayer(const TSoftClassPtr<UUserWidget>& InWidget, ENsUILayerType InLayerType)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        return UISubsystem->ReplaceTopWidgetOnLayer(InWidget, InLayerType);
    }
    return nullptr;
}

void UNsUILibrary::PopWidgetFromLayer(ENsUILayerType InLayerType)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->PopWidgetFromLayer(InLayerType);
    }
}

void UNsUILibrary::RemoveWidgetFromLayer(ENsUILayerType InLayerType, UUserWidget* InWidget)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->RemoveWidgetFromLayer(InLayerType, InWidget);
    }
}

void UNsUILibrary::ClearLayer(ENsUILayerType InLayerType)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->ClearLayer(InLayerType);
    }
}

UUserWidget* UNsUILibrary::ApplyLayerTransition(const FNsUILayerTransitionRequest& Request)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        if (UNsUIWidgetStackElement* Result = UISubsystem->ApplyLayerTransition(Request))
        {
            return Result->Widget.Get();
        }
    }
    return nullptr;
}

void UNsUILibrary::GatherAllNavigableDescendants(const UUserWidget* RootWidget, TArray<UWidget*>& OutNavigables)
{
    if (!RootWidget)
    {
        return;
    }

    TArray<UWidget*> Stack;
    RootWidget->WidgetTree->GetAllWidgets(Stack);

    for (UWidget* Widget : Stack)
    {
        if (!Widget)
        {
            continue;
        }

        if (Widget->bIsEnabled && Widget->GetIsEnabled())
        {
            if (Widget->IsVisible() && Widget->SupportsKeyboardFocus())
            {
                OutNavigables.Add(Widget);
            }
        }
    }
}

UUserWidget* UNsUILibrary::FindLayerContentRoot(UWidget* InWidget)
{
    if (!InWidget)
    {
        return nullptr;
    }

    UWidget* Current = InWidget;
    while (Current)
    {
        if (UNsUILayer* AsLayer = Cast<UNsUILayer>(Current->GetOuter()))
        {
            return Cast<UUserWidget>(Current);
        }

        if (UWidgetTree* Tree = Current->WidgetTree)
        {
            Current = Tree->RootWidget;
        }
        else
        {
            Current = nullptr;
        }
    }

    return nullptr;
}
