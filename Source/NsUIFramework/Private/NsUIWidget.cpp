// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUIWidget.h"

#include "NsUILibrary.h"
#include "NsUISubsystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Components/WidgetNavigation.h"

UNsUIWidget::UNsUIWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsFocusable(true);
}

void UNsUIWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    ApplyNeighborNavigation();
}

FReply UNsUIWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->NotifyWidgetFocused(this);
    }
    return Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
}

void UNsUIWidget::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->NotifyWidgetFocusLost(this);
    }
    Super::NativeOnFocusLost(InFocusEvent);
}

void UNsUIWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->NotifyWidgetFocused(this);
    }
    Super::NativeOnAddedToFocusPath(InFocusEvent);
}

void UNsUIWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->NotifyWidgetFocusLost(this);
    }
    Super::NativeOnRemovedFromFocusPath(InFocusEvent);
}

UWidget* UNsUIWidget::GetNeighbor(const EUINavigation Direction) const
{
    switch (Direction)
    {
        case EUINavigation::Up:    return UpNeighbor.IsValid() ? UpNeighbor.Get() : nullptr;
        case EUINavigation::Down:  return DownNeighbor.IsValid() ? DownNeighbor.Get() : nullptr;
        case EUINavigation::Left:  return LeftNeighbor.IsValid() ? LeftNeighbor.Get() : nullptr;
        case EUINavigation::Right: return RightNeighbor.IsValid() ? RightNeighbor.Get() : nullptr;
        default:                   return nullptr;
    }
}

void UNsUIWidget::ApplyNeighborNavigation()
{
    if (UWidgetNavigation* Navigation = WidgetNavigation)
    {
        Navigation->ClearNavigationData();

        auto SetExplicit = [Navigation](EUINavigation Dir, const TSoftObjectPtr<UWidget>& Target)
        {
            if (Target.IsValid())
            {
                FWidgetNavigationData* NavData = nullptr;
                switch (Dir)
                {
                    case EUINavigation::Up:    NavData = &Navigation->Up; break;
                    case EUINavigation::Down:  NavData = &Navigation->Down; break;
                    case EUINavigation::Left:  NavData = &Navigation->Left; break;
                    case EUINavigation::Right: NavData = &Navigation->Right; break;
                    default: break;
                }

                if (NavData != nullptr)
                {
                    NavData->Rule = EUINavigationRule::Explicit;
                    NavData->Widget = Target;
                }
            }
        };

        SetExplicit(EUINavigation::Up, UpNeighbor);
        SetExplicit(EUINavigation::Down, DownNeighbor);
        SetExplicit(EUINavigation::Left, LeftNeighbor);
        SetExplicit(EUINavigation::Right, RightNeighbor);
    }
}

void UNsUIWidget::HandleUIConfirm_Implementation()
{
}

void UNsUIWidget::HandleUICancel_Implementation()
{
}

void UNsUIWidget::OnWidgetActivated_Implementation()
{
}

void UNsUIWidget::OnWidgetDeactivated_Implementation()
{
}

UWidget* UNsUIWidget::GetInitialFocusWidget_Implementation() const
{
    TArray<UWidget*> NavigableWidgets;
    UNsUILibrary::GatherAllNavigableDescendants(this, NavigableWidgets);
    return NavigableWidgets.IsValidIndex(0) ? NavigableWidgets[0] : const_cast<UNsUIWidget*>(this);
}
