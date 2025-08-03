#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "INsUINavigation.h"
#include "NsUILibrary.h"
#include "NsUISubsystem.h"
#include "NsUIWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class NSUIFRAMEWORK_API UNsUIWidget : public UUserWidget, public INsUINavigation
{
    GENERATED_BODY()

    virtual void NativeConstruct() override
    {
        SetIsFocusable(true);
    }

// Functions
public:

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

    /** On widget pushed */
    virtual void OnWidgetPushed_Implementation() override
    {
        // Default: try to focus the first element that is found
        TArray<class UWidget*> NavigableWidgets; UNsUILibrary::GatherAllNavigableDescendants(this, NavigableWidgets);
        if (UNsUISubsystem* const UISubsystem = UNsUISubsystem::GetSubsystem())
        {
            // if we do other widgets that we can focus inside, auto-focus the first one, if not we will focus the widget itself so he can control take control
            // in these cases where we dont have any other widgets inside to focus we should override our functions to be able to restore the control
            UISubsystem->SetCurrentFocusedWidget(NavigableWidgets.IsValidIndex(0) ? NavigableWidgets[0] : this);
        }
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