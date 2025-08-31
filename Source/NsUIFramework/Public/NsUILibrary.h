// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsUISubsystem.h"
#include "NsUILibrary.generated.h"

/**
 * Night Shift UI Type Library
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUILibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /** Add widget to layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework")
    static class UUserWidget* AddWidgetToLayer(const TSoftClassPtr<class UUserWidget>& InWidget, const ENsUILayerType InLayerType);

    /** Pop widget from layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework")
    static void PopWidgetFromLayer(const ENsUILayerType InLayerType);

    /** Gather all navigable widgets inside another widget */
    UFUNCTION()
    static void GatherAllNavigableDescendants(const class UUserWidget* const InRootWidget, TArray<class UWidget*>& InOutNavigables);

    /**
     * Find the "content root" for navigation:
     * - the outermost UUserWidget that is still *inside* an UNsUILayer
     * This is the widget that the layer pushed on its stack (e.g., Inventory), not a nested cell/user widget (e.g., Item) and not the whole layout.
     */
    UFUNCTION()
    static UUserWidget* FindLayerContentRoot(UWidget* InWidget);
    
    /**
     * Convenience helper to register actors for selection and bind callbacks.
     * Clears any previously registered actors and delegate bindings, projects
     * each actor's bounds to screen space and registers them with the UI
     * subsystem.
     *
     * @param Actors     Actors that should become selectable.
     * @param OnConfirm  Required delegate executed when the focused actor is confirmed.
     * @param OnCancel   Optional delegate executed when selection is cancelled.
     * @param OnToggle   Optional delegate executed when the current actor is toggled.
     *                  Leave unbound if you do not need multi-selection support.
     */
    static void RegisterActorsForSelection(const TArray<AActor*>& Actors,
        const FNsUIActorDelegate::FDelegate& OnConfirm,
        const FNsUIActorDelegate::FDelegate& OnCancel = FNsUIActorDelegate::FDelegate(),
        const FNsUIActorDelegate::FDelegate& OnToggle = FNsUIActorDelegate::FDelegate());
};