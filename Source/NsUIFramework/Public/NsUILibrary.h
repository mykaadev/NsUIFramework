// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsUILibrary.generated.h"

/**
 * Enum used to define the layer type
 */
UENUM(BlueprintType)
enum class ENsUILayerType : uint8
{
    None UMETA(Hidden),
    GameStatic = 0,
    GameMenu = 1,
    GameModal = 2,
    ApplicationStatic = 3,
    ApplicationMenu = 4,
    ApplicationModal = 5
};

/** Minimal layer transition operations so all stack changes are centralized */
UENUM(BlueprintType)
enum class ENsUILayerTransitionOp : uint8
{
    Push,
    ReplaceTop,
    Pop,
    Remove,
    ClearLayer,
    Unwind
};

/** Simple request used by ApplyLayerTransition */
USTRUCT(BlueprintType)
struct FNsUILayerTransitionRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ns UI Framework")
    ENsUILayerType LayerType = ENsUILayerType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ns UI Framework")
    ENsUILayerTransitionOp Operation = ENsUILayerTransitionOp::Push;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ns UI Framework")
    TSoftClassPtr<class UUserWidget> WidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ns UI Framework")
    TObjectPtr<class UUserWidget> WidgetInstance = nullptr;

    /** When pushing, choose whether to collapse the previous widget. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ns UI Framework")
    bool bCollapsePrevious = true;
};

/** Lightweight stack element that only tracks the widget and owning layer */
UCLASS()
class UNsUIWidgetStackElement : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<class UUserWidget> Widget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    ENsUILayerType Layer = ENsUILayerType::None;
};

/**
 * Nulled Softworks UI Type Library
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUILibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Push a widget to the given layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework")
    static class UUserWidget* AddWidgetToLayer(const TSoftClassPtr<class UUserWidget>& InWidget, ENsUILayerType InLayerType, bool bCollapsePrevious = true);

    /** Replace the top widget on a layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework")
    static class UUserWidget* ReplaceTopWidgetOnLayer(const TSoftClassPtr<class UUserWidget>& InWidget, ENsUILayerType InLayerType);

    /** Pop the top widget from a layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework")
    static void PopWidgetFromLayer(ENsUILayerType InLayerType);

    /** Pop everything from a layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework")
    static void UnwindLayer(ENsUILayerType InLayerType);

    /** Remove a specific widget from a layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework")
    static void RemoveWidgetFromLayer(ENsUILayerType InLayerType, class UUserWidget* InWidget);

    /** Clear every widget from a layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework")
    static void ClearLayer(ENsUILayerType InLayerType);

    /** Perform a centralized layer transition (Push/Replace/Pop/Remove/Clear) */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework")
    static class UUserWidget* ApplyLayerTransition(const FNsUILayerTransitionRequest& Request);

    /** Gather all navigable widgets inside another widget */
    UFUNCTION()
    static void GatherAllNavigableDescendants(const class UUserWidget* RootWidget, TArray<class UWidget*>& OutNavigables);

    /**
     * Find the content root for navigation. Returns the outermost widget that
     * lives inside an NsUILayer.
     */
    UFUNCTION()
    static UUserWidget* FindLayerContentRoot(UWidget* InWidget);
};
