// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NsUILibrary.h"
#include "Tickable.h"
#include "NsUISubsystem.generated.h"

/**
 * Nulled Softworks UI Subsystem
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUISubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

// Functions
public:

    /** Constructor */
    UNsUISubsystem();

    //~ Begin FTickableGameObject Interface
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return bActorSelectionActive; }
    //~ End FTickableGameObject Interface

    //~ Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface

    /** Get UI Subsystem */
    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    static UNsUISubsystem* GetSubsystem();

    /** Push widget to layer */
    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    class UUserWidget* PushWidgetToLayer(const TSoftClassPtr<class UUserWidget>& InWidget, const ENsUILayerType InLayerType);

    /** Pop widget to layer */
    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void PopWidgetFromLayer(const ENsUILayerType InLayerType);

    /** Remove a specific widget from a layer without popping blindly */
    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void RemoveWidgetFromLayer(const ENsUILayerType InLayerType, class UUserWidget* InWidget);

    /** Set Active UI Layout */
    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void SetActiveUILayout(const TSoftClassPtr<class UNsUILayout>& InLayout);

    /** Get Active UI Layout */
    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    UNsUILayout* GetActiveUILayout() const;

    /**
     * Called when the player presses the navigation input (eg: WASD)
     * @param Direction  A 2D vector: (-1,0)=Left, (1,0)=Right, (0,-1)=Up, (0,1)=Down
     */
    void HandleNavigate(const FVector2D& Direction);

    /** Called when the player presses the Confirm input */
    bool HandleConfirm();

    /** Called when the player presses the Cancel input */
    bool HandleCancel();

    /** Called when the player presses a toggle selection input */
    void HandleToggle();

    /** Enable world-actor selection mode */
    void EnterActorSelectionMode();

    /** Disable world-actor selection mode */
    void ExitActorSelectionMode();

    /** Register a world actor as a selectable focus target */
    int32 RegisterActorFocus(AActor* Actor, const FBox2D& ScreenBounds);

    /** Remove a previously registered actor */
    void UnregisterActorFocus(AActor* Actor);

    /** Remove all registered actors */
    void ClearActorFocusTargets();

    /** Event fired when an actor is confirmed */
    FNsUIActorDelegate OnActorConfirmed;

    /** Event fired when an actor is cancelled */
    FNsUIActorDelegate OnActorCancelled;

    /** Event fired when an actor is toggled */
    FNsUIActorDelegate OnActorToggled;

    /**
    * If a widget’s explicit neighbor (e.g. GetUpNeighbor()) is null, we fall back
    * to this: scan all other widgets in the same layer (via the UI Subsystem) and
    * pick the “best” candidate by geometry.
    *
    * @param InCurrentWidget  The widget that currently has focus.
    * @param InNavDir         Which direction we’re moving (Up/Down/Left/Right).
    * @return A UWidget* pointing at the best neighbor in that direction, or nullptr if none.
    */
    class UWidget* FindBestNeighborBySpatialSearch(class UWidget* const InCurrentWidget, const EUINavigation InNavDir) const;

    /** Get Widget geometry center */
    FVector2D GetWidgetCenterInViewport(const class UWidget* const InWidget) const;

    /** Explicitly set which widget is currently “focused” in the UI, called by the layer when it pushes or pops a widget */
    void SetCurrentFocusedWidget(class UWidget* const InNewFocus);

    /** Get the current focused widget */
    class UWidget* GetCurrentFocusedWidget() const;

    /** Helper: if CurrentFocusedWidget implements INsUINavigable, return a pointer to that interface */
    class INsUINavigation* GetCurrentNavInterface() const;

    /** Called once at subsystem init to fill the maps */
    void InitializeCommandRegistry();

    /** Move the cursor to the currently focused actor */
    void UpdateActorCursor();

    /** Get current layer stack scope */
    UNsUIWidgetStackElement* GetCurrentScope();

// Variables
private:

    /** Instance of the cursor widget */
    UPROPERTY()
    TObjectPtr<class UNsUICursor> CursorWidget;

    /** Active UI layout */
    UPROPERTY(VisibleDefaultsOnly)
    TObjectPtr<class UNsUILayout> ActiveUILayout;

    /** Current layer stack scope */
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class UNsUIWidgetStackElement> CurrentScope;

    /** Map from widget class to what to do when HandleConfirm() is pressed */
    TMap<UClass*, FWidgetCommand> ConfirmRegistry;

    /** Map from widget class to what to do when HandleCancel() is pressed */
    TMap<UClass*, FWidgetCommand> CancelRegistry;

    /** List of registered world-actor focus targets */
    TArray<FNsUIActorFocusInfo> ActorFocusTargets;

    /** Index of currently focused actor */
    int32 CurrentActorIndex;

    /** Whether we are in actor-selection mode */
    bool bActorSelectionActive;

    /** Find the next actor relative to current one based on navigation direction */
    int32 FindBestActorBySpatialSearch(int32 CurrentIndex, EUINavigation NavDir) const;

    /** Get center of actor's screen-space bounds */
    FVector2D GetActorScreenCenter(const FNsUIActorFocusInfo& Info) const;
};
