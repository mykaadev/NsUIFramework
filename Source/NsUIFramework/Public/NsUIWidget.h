// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "INsUINavigation.h"
#include "NsUILibrary.h"
#include "NsUIWidget.generated.h"

/**
 * Basic UI Widget
 */
UCLASS(Abstract, Blueprintable)
class NSUIFRAMEWORK_API UNsUIWidget : public UUserWidget, public INsUINavigation
{
 
    GENERATED_BODY()

// Functions
public:

    virtual void NativeConstruct() override
    {
        SetIsFocusable(true);
    }


    /** Get up neighbor */
    virtual class UWidget* GetUpNeighbor_Implementation() override
    {
        return UpNeighbor.IsValid() ? UpNeighbor.Get() : nullptr;
    }

    /** Get down neighbor */
    virtual class UWidget* GetDownNeighbor_Implementation() override
    {
        return DownNeighbor.IsValid() ? DownNeighbor.Get() : nullptr;
    }

    /** Get right neighbor */
    virtual class UWidget* GetLeftNeighbor_Implementation() override
    {
        return LeftNeighbor.IsValid() ? LeftNeighbor.Get() : nullptr;
    }

    /** Get right neighbor */
    virtual class UWidget* GetRightNeighbor_Implementation() override
    {
        return RightNeighbor.IsValid() ? RightNeighbor.Get() : nullptr;
    }

    /** Handle UI Confirm */
    virtual void HandleUIConfirm_Implementation() override
    {
        // Default: do nothing. Override in Blueprint or subclass to actually “click” or “activate” something.
    }

    /** Handle UI Cancel */
    virtual void HandleUICancel_Implementation() override
    {
        // Default: do nothing. You can override to close a menu, etc.
    }

    /** On widget pushed - runs once we push a widget to the stack */
    virtual void OnWidgetPushed_Implementation() override
    {
        // Default: do nothing. You can override to close a menu, etc.
    }

    /** Called by default after pushing a widget this can setup the initial focused widget of a widget */
    virtual UWidget* GetInitialFocusWidget_Implementation() override
    {
        // Default: try to focus the first element that is found
        // if we have other widgets that we can focused inside, auto-focus the first one,
        TArray<class UWidget*> NavigableWidgets; UNsUILibrary::GatherAllNavigableDescendants(this, NavigableWidgets);
        return NavigableWidgets.IsValidIndex(0) ? NavigableWidgets[0] : this;
    }

// Variables
public:

    /** Up neighbor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    TSoftObjectPtr<class UWidget> UpNeighbor;

    /** Down neighbor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    TSoftObjectPtr<class UWidget> DownNeighbor;

    /** Left neighbor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    TSoftObjectPtr<class UWidget> LeftNeighbor;

    /** Right neighbor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    TSoftObjectPtr<class UWidget> RightNeighbor;
};