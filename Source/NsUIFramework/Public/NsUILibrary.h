// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsUILibrary.generated.h"

/** Enum used to define the layer type */
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

/** Lifetime policy for widgets living on a layer. */
UENUM(BlueprintType)
enum class ENsUIWidgetLifetime : uint8
{
    Transient UMETA(DisplayName = "Transient (auto-pop on blur)"),
    Modal UMETA(DisplayName = "Modal (blocks lower layers)"),
    Persistent UMETA(DisplayName = "Persistent (HUD)"),
    Pinned UMETA(DisplayName = "Pinned (cannot be removed except by tag/ID)")
};

/** Layer priority used when resolving focus/input across stacks. Higher wins. */
UENUM(BlueprintType)
enum class ENsUILayerPriority : uint8
{
    HUD = 0,
    Menu = 1,
    Dialog = 2,
    Overlay = 3,
    Debug = 4
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
    Unwind,
    ClearAll
};

/** Deterministic focus rules for transitions. */
UENUM(BlueprintType)
enum class ENsUIFocusRule : uint8
{
    Auto,
    SetInitial,
    RestorePrevious,
    None
};

/** Basic state event emitted for every stack change. */
USTRUCT(BlueprintType)
struct FNsUIStateEvent
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework")
    ENsUILayerType Layer = ENsUILayerType::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework")
    ENsUILayerTransitionOp Operation = ENsUILayerTransitionOp::Push;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework")
    TWeakObjectPtr<class UUserWidget> Widget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework")
    FGuid StableId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework")
    TArray<FName> Tags;
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

    /** Optional stable identifier for targeted removal or restore. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ns UI Framework")
    FGuid StableId;

    /** Tags applied to the stack element for later queries/removal. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ns UI Framework")
    TArray<FName> Tags;

    /** How long this widget should live and whether it blocks input to lower layers. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ns UI Framework")
    ENsUIWidgetLifetime Lifetime = ENsUIWidgetLifetime::Transient;

    /** Focus rule to apply after the transition. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ns UI Framework")
    ENsUIFocusRule FocusRule = ENsUIFocusRule::Auto;

    /** When pushing, choose whether to collapse the previous widget. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ns UI Framework")
    bool bCollapsePrevious = true;

    /** When true, previous widget remains visible if the layer has a panel root. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ns UI Framework")
    bool bKeepPreviousVisible = false;
};

/** Lightweight stack element that tracks the widget and whether it collapsed the entry below. */
UCLASS()
class NSUIFRAMEWORK_API UNsUIWidgetStackElement : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework|Layer")
    TObjectPtr<class UUserWidget> Widget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework|Layer")
    FGuid StableId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework|Layer")
    TArray<FName> Tags;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework|Layer")
    ENsUILayerType Layer = ENsUILayerType::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework|Layer")
    bool bCollapsedEntryBelow = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework|Layer")
    ENsUIWidgetLifetime Lifetime = ENsUIWidgetLifetime::Transient;
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
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layers")
    static class UUserWidget* AddWidgetToLayer(const TSoftClassPtr<class UUserWidget>& InWidget, ENsUILayerType InLayerType, bool bCollapsePrevious = true);

    /** Replace the top widget on a layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layers")
    static class UUserWidget* ReplaceTopWidgetOnLayer(const TSoftClassPtr<class UUserWidget>& InWidget, ENsUILayerType InLayerType, bool bCollapsePrevious = true);

    /** Pop the top widget from a layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layers")
    static void PopWidgetFromLayer(ENsUILayerType InLayerType);

    /** Pop everything from a layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layers")
    static void UnwindLayer(ENsUILayerType InLayerType);

    /** Remove a specific widget from a layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layers")
    static void RemoveWidgetFromLayer(ENsUILayerType InLayerType, class UUserWidget* InWidget);

    /** Clear every widget from a layer */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layers")
    static void ClearLayer(ENsUILayerType InLayerType);

    /** Clear every layer and stack. */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layers")
    static void ClearAllLayers();

    /** Perform a centralized layer transition (Push/Replace/Pop/Remove/Clear/Unwind) */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layers")
    static class UUserWidget* ApplyLayerTransition(const FNsUILayerTransitionRequest& Request);

    /** Swap the active layout used by the subsystem. */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layouts")
    static void SetActiveLayout(const TSoftClassPtr<class UNsUILayout>& InLayoutClass);

    /** Retro cursor helpers */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Cursor")
    static void SetRetroCursorClass(const TSoftClassPtr<class UNsUICursor>& InCursorClass);

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Cursor")
    static void MoveRetroCursorToWidget(class UWidget* TargetWidget);

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Cursor")
    static void MoveRetroCursorToScreenPosition(const FVector2D& ScreenPosition);

    /** Actor selection helpers that also drive the retro cursor when requested. */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Actor Selection")
    static void StartActorSelection(const TArray<TSoftObjectPtr<class AActor>>& Actors, bool bSnapCursorToSelection = true);

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Actor Selection")
    static void ExitActorSelection();

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Actor Selection")
    static class AActor* StepActorSelection(int32 Direction, bool bSnapCursorToSelection = true);

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Actor Selection")
    static class AActor* GetCurrentActorSelection();

    /** Access the most recent UI state events (for debugging/telemetry). */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Telemetry")
    static const TArray<FNsUIStateEvent>& GetRecentStateEvents();

    /** Gather all navigable widgets inside another widget */
    UFUNCTION()
    static void GatherAllNavigableDescendants(const class UUserWidget* RootWidget, TArray<class UWidget*>& OutNavigables);

    /** Find the content root for navigation. Returns the outermost widget that lives inside an NsUILayer. */
    UFUNCTION()
    static UUserWidget* FindLayerContentRoot(UWidget* InWidget);
};
