// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUILibrary.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Components/PanelWidget.h"
#include "NsUISubsystem.h"
#include "NsUIWidget.h"

void UNsUILibrary::GatherAllNavigableDescendants(const UUserWidget* InRootWidget, TArray<UWidget*>& InOutNavigables)
{
    if (!IsValid(InRootWidget) || !IsValid(InRootWidget->WidgetTree))
    {
        return;
    }

    TArray<UWidget*> Pending;
    Pending.Add(InRootWidget->WidgetTree->RootWidget);

    while (Pending.Num() > 0)
    {
        UWidget* Widget = Pending.Pop();
        if (!IsValid(Widget))
        {
            continue;
        }

        if (Widget->GetIsEnabled() && Widget->IsVisible() && Widget->SupportsKeyboardFocus())
        {
            InOutNavigables.Add(Widget);
        }

        if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
        {
            const int32 ChildCount = Panel->GetChildrenCount();
            for (int32 Index = 0; Index < ChildCount; ++Index)
            {
                Pending.Add(Panel->GetChildAt(Index));
            }
        }
        else if (UUserWidget* NestedUserWidget = Cast<UUserWidget>(Widget))
        {
            if (NestedUserWidget->WidgetTree && NestedUserWidget->WidgetTree->RootWidget)
            {
                Pending.Add(NestedUserWidget->WidgetTree->RootWidget);
            }
        }
    }
}

UWidget* UNsUILibrary::FindFirstFocusableDescendant(const UUserWidget* InRootWidget)
{
    TArray<UWidget*> Navigables;
    GatherAllNavigableDescendants(InRootWidget, Navigables);
    return Navigables.IsValidIndex(0) ? Navigables[0] : nullptr;
}

namespace
{
static const FName GameStaticLayerName(TEXT("GameStatic"));
static const FName GameMenuLayerName(TEXT("GameMenu"));
static const FName GameModalLayerName(TEXT("GameModal"));
static const FName ApplicationStaticLayerName(TEXT("ApplicationStatic"));
static const FName ApplicationMenuLayerName(TEXT("ApplicationMenu"));
static const FName ApplicationModalLayerName(TEXT("ApplicationModal"));
}

FName UNsUILibrary::GetLayerName(ENsUILayer Layer)
{
    switch (Layer)
    {
    case ENsUILayer::GameStatic:
        return GameStaticLayerName;
    case ENsUILayer::GameMenu:
        return GameMenuLayerName;
    case ENsUILayer::GameModal:
        return GameModalLayerName;
    case ENsUILayer::ApplicationStatic:
        return ApplicationStaticLayerName;
    case ENsUILayer::ApplicationMenu:
        return ApplicationMenuLayerName;
    case ENsUILayer::ApplicationModal:
        return ApplicationModalLayerName;
    default:
        return NAME_None;
    }
}

UNsUIWidget* UNsUILibrary::PushWidgetToLayer(TSubclassOf<UNsUIWidget> WidgetClass, ENsUILayer Layer)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        return Subsystem->PushWidget(WidgetClass, GetLayerName(Layer));
    }
    return nullptr;
}

bool UNsUILibrary::PushWidgetInstanceToLayer(UNsUIWidget* Widget, ENsUILayer Layer)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        return Subsystem->PushWidgetInstance(Widget, GetLayerName(Layer));
    }
    return false;
}

void UNsUILibrary::PopLayer(ENsUILayer Layer)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->PopWidget(GetLayerName(Layer));
    }
}

void UNsUILibrary::PopLayerToRoot(ENsUILayer Layer)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->PopToRoot(GetLayerName(Layer));
    }
}

void UNsUILibrary::RegisterActorsForSelection(const TArray<AActor*>& Actors,
                                              const FNsUIActorDelegate::FDelegate& OnConfirm,
                                              const FNsUIActorDelegate::FDelegate& OnCancel,
                                              const FNsUIActorDelegate::FDelegate& OnToggle)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->ExitActorSelectionMode();

        Subsystem->OnActorConfirmed.Clear();
        Subsystem->OnActorCancelled.Clear();
        Subsystem->OnActorToggled.Clear();

        Subsystem->OnActorConfirmed.Add(OnConfirm);
        if (OnCancel.IsBound())
        {
            Subsystem->OnActorCancelled.Add(OnCancel);
        }
        if (OnToggle.IsBound())
        {
            Subsystem->OnActorToggled.Add(OnToggle);
        }

        Subsystem->EnterActorSelectionMode(Actors);
    }
}
