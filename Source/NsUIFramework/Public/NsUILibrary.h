// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
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

    /** Walks the outer chain of a UWidget to find the UUserWidget that owns it (via its WidgetTree) */
    UFUNCTION()
    static UUserWidget* FindRootUserWidget(class UWidget* const InWidget);
};