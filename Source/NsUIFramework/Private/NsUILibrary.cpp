// Copyright (C) 2024 mykaa. All rights reserved.

#include "NsUILibrary.h"
#include "Blueprint/WidgetTree.h"
#include "INsUINavigation.h"
#include "NsUILayout.h"
#include "NsUISubsystem.h"

UUserWidget* UNsUILibrary::AddWidgetToLayer(const TSoftClassPtr<class UUserWidget>& InWidget, const ENsUILayerType InLayerType)
{
    if (UNsUISubsystem* const UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        return UISubsystem->PushWidgetToLayer(InWidget, InLayerType);
    }

    return nullptr;
}

void UNsUILibrary::PopWidgetFromLayer(const ENsUILayerType InLayerType)
{
    if (UNsUISubsystem* const UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->PopWidgetFromLayer(InLayerType);
    }
}

UUserWidget* UNsUILibrary::FindRootUserWidget(UWidget* const InWidget)
{
    if (InWidget == nullptr)
    {
        return nullptr;
    }

    UUserWidget* LastFound = nullptr;
    UObject* Outer = InWidget;

    // Walk up until we run out of outers.
    while (Outer != nullptr)
    {
        if (UUserWidget* const UserWidget = Cast<UUserWidget>(Outer))
        {
            // Record each UUserWidget we see
            LastFound = UserWidget;
        }

        Outer = Outer->GetOuter();
    }

    // 'LastFound' is the topmost UUserWidget
    return LastFound;
}

static bool CanWidgetReceiveFocus(const UWidget* Widget)
{
    if (!Widget)
    {
        return false;
    }

    // Make sure the UWidget has already constructed its Slate counterpart
    const TSharedPtr<SWidget> SlateWidget = Widget->GetCachedWidget();
    if (!SlateWidget.IsValid())
    {
        return false;
    }

    // Slate’s SupportsKeyboardFocus tells you if it can ever receive focus
    return SlateWidget->SupportsKeyboardFocus();
}

void UNsUILibrary::GatherAllNavigableDescendants(const UUserWidget* const InRootWidget, TArray<UWidget*>& InOutNavigables)
{
    if (!InRootWidget || !InRootWidget->WidgetTree)
    {
        return;
    }

    TArray<UWidget*> AllWidgets;
    InRootWidget->WidgetTree->GetAllWidgets(AllWidgets);

    for (UWidget* Widget : AllWidgets)
    {
        if (!Widget || !Widget->IsVisible() || !Widget->GetIsEnabled())
        {
            continue;
        }

        // 1) Engine‐defined focusable?
        const bool bSlateFocusable = CanWidgetReceiveFocus(Widget);

        // 2) Your custom interface?
        const bool bCustomNav = Widget->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass());

        if (bSlateFocusable || bCustomNav)
        {
            InOutNavigables.Emplace(Widget);
        }
    }
}
