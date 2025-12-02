// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUILibrary.h"

#include "Blueprint/WidgetTree.h"
#include "Engine/Engine.h"
#include "NsUILayout.h"
#include "NsUILayer.h"
#include "NsUISubsystem.h"
#include "NsUIWidget.h"

UUserWidget* UNsUILibrary::AddWidgetToLayer(const TSoftClassPtr<UUserWidget>& InWidget, ENsUILayerType InLayerType, bool bCollapsePrevious)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        FNsUILayerTransitionRequest Request;
        Request.LayerType = InLayerType;
        Request.Operation = ENsUILayerTransitionOp::Push;
        Request.WidgetClass = InWidget;
        Request.bCollapsePrevious = bCollapsePrevious;
        return UISubsystem->PushWidgetToLayer(InWidget, InLayerType, Request);
    }
    return nullptr;
}

UUserWidget* UNsUILibrary::ReplaceTopWidgetOnLayer(const TSoftClassPtr<UUserWidget>& InWidget, ENsUILayerType InLayerType, bool bCollapsePrevious)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        FNsUILayerTransitionRequest Request;
        Request.LayerType = InLayerType;
        Request.Operation = ENsUILayerTransitionOp::ReplaceTop;
        Request.WidgetClass = InWidget;
        Request.bCollapsePrevious = bCollapsePrevious;
        return UISubsystem->ReplaceTopWidgetOnLayer(InWidget, InLayerType, Request);
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

void UNsUILibrary::UnwindLayer(ENsUILayerType InLayerType)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->UnwindLayer(InLayerType);
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

void UNsUILibrary::ClearAllLayers()
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->ClearAllLayers();
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

void UNsUILibrary::SetActiveLayout(const TSoftClassPtr<UNsUILayout>& InLayoutClass)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->SetActiveUILayout(InLayoutClass);
    }
}

void UNsUILibrary::SetRetroCursorClass(const TSoftClassPtr<UNsUICursor>& InCursorClass)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->SetRetroCursorClass(InCursorClass);
    }
}

void UNsUILibrary::MoveRetroCursorToWidget(UWidget* TargetWidget)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->MoveRetroCursorToWidget(TargetWidget);
    }
}

void UNsUILibrary::MoveRetroCursorToScreenPosition(const FVector2D& ScreenPosition)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->MoveRetroCursorToScreenPosition(ScreenPosition);
    }
}

void UNsUILibrary::StartActorSelection(const TArray<TSoftObjectPtr<AActor>>& Actors, bool bSnapCursorToSelection)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->StartActorSelectionSession(Actors, bSnapCursorToSelection);
    }
}

void UNsUILibrary::ExitActorSelection()
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->ExitActorSelectionSession();
    }
}

AActor* UNsUILibrary::StepActorSelection(int32 Direction, bool bSnapCursorToSelection)
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        return UISubsystem->StepActorSelection(Direction, bSnapCursorToSelection);
    }
    return nullptr;
}

AActor* UNsUILibrary::GetCurrentActorSelection()
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        return UISubsystem->GetCurrentActorSelection();
    }
    return nullptr;
}

const TArray<FNsUIStateEvent>& UNsUILibrary::GetRecentStateEvents()
{
    if (UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        return UISubsystem->GetRecentStateEvents();
    }

    static const TArray<FNsUIStateEvent> Empty;
    return Empty;
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
