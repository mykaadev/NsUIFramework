// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsUILibrary.generated.h"

/** A little alias for readability */
using FWidgetCommand = TFunction<void(class UWidget*)>;

/** Delegate used to notify about actor interactions */
DECLARE_MULTICAST_DELEGATE_OneParam(FNsUIActorDelegate, AActor*);

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

/**
 * Object to hold the info from each layer stack element
 */
UCLASS()
class UNsUIWidgetStackElement : public UObject
{

    GENERATED_BODY()
    
// Variables
public:

    /** Current widget scope (e.g. the exit confirmation menu) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<class UUserWidget> WidgetScope;

    /** Current focused widget element (e.g. the "yes" button from the exit confirmation menu) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<class UWidget> WidgetScopeFocusedElement;

    /** The layer where the widget scope resides */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    ENsUILayerType WidgetScopeLayerType;

    /** The actual widget that contained the invoker element (e.g. the pause menu) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<class UUserWidget> InvokerWidgetScope;

    /** The widget that triggered a push onto this layer (e.g., "Exit" button in Menu). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<class UWidget> InvokerElement;

    /** The layer where the invoker resides */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    ENsUILayerType InvokerLayerType;

// Functions
public:

    //bool operator ==(const UNsUIWidgetStackElement& InRHS)
    //{
    //    return WidgetScope == InRHS.WidgetScope &&
    //            WidgetScopeFocusedElement == InRHS.WidgetScopeFocusedElement &&
    //            WidgetScopeLayerType == InRHS.WidgetScopeLayerType &&
    //            InvokerWidgetScope == InRHS.InvokerWidgetScope &&
    //            InvokerElement == InRHS.InvokerElement &&
    //            InvokerLayerType == InRHS.InvokerLayerType;
    //}

    /** Constructor */
    UNsUIWidgetStackElement()
        : WidgetScope(nullptr)
        , WidgetScopeFocusedElement(nullptr)
        , WidgetScopeLayerType(ENsUILayerType::None)
        , InvokerWidgetScope(nullptr)
        , InvokerElement(nullptr)
        , InvokerLayerType(ENsUILayerType::None)
    {
    }

    /** Constructor */
    UNsUIWidgetStackElement(const TObjectPtr<class UUserWidget> InWidgetScope
                            , const TObjectPtr<class UWidget> InWidgetScopeFocusedElement
                            , const ENsUILayerType InWidgetScopeLayerType
                            , const TObjectPtr<class UUserWidget> InInvokerWidgetScope
                            , const TObjectPtr<class UWidget> InvokerElement
                            , const ENsUILayerType InInvokerLayerType)
        : WidgetScope(InWidgetScope)
        , WidgetScopeFocusedElement(InWidgetScopeFocusedElement)
        , WidgetScopeLayerType(InWidgetScopeLayerType)
        , InvokerWidgetScope(InInvokerWidgetScope)
        , InvokerElement(InvokerElement)
        , InvokerLayerType(InInvokerLayerType)
    {
    }

    /** Destructor */
    ~UNsUIWidgetStackElement()
    {
        WidgetScope = nullptr;
        WidgetScopeFocusedElement = nullptr;
        InvokerWidgetScope = nullptr;
        InvokerElement = nullptr;
    }
};

/** Holds info for a world actor registered as a focus target. */
struct FNsUIActorFocusInfo
{
    TWeakObjectPtr<AActor> Actor;
    /** Actor's projected bounds in viewport pixel coordinates */
    FBox2D ScreenBounds;
};

/**
 * Nulled Softworks UI Type Library
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

    /** Remove a specific widget from a layer without popping blindly */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework")
    static void RemoveWidgetFromLayer(const ENsUILayerType InLayerType, class UUserWidget* InWidget);

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
