// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NsUICursor.h"
#include "NsUISubsystem.generated.h"

/** A little alias for readability */
using FWidgetCommand = TFunction<void(class UWidget*)>;

/**
 * Night Shift UI Subsystem
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUISubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

// Functions
public:

    /** Constructor */
    UNsUISubsystem();

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

    /** Helper: if CurrentFocusedWidget implements INsUINavigable, return a pointer to that interface */
    class INsUINavigation* GetCurrentNavInterface() const;

    /** Called once at subsystem init to fill the maps */
    void InitializeCommandRegistry();

// Variables
private:

    /** Instance of the cursor widget */
    UPROPERTY()
    TObjectPtr<class UNsUICursor> CursorWidget;

    /** Active UI layout */
    UPROPERTY(VisibleDefaultsOnly)
    TObjectPtr<class UNsUILayout> ActiveUILayout;

    /** A weak pointer to the currently focused widget. */
    TWeakObjectPtr<UWidget> CurrentFocusedWidget;

    /** A weak pointer to the previous focused widget. */
    TWeakObjectPtr<UWidget> PreviousFocusedWidget;

    /** Map from widget class to what to do when HandleConfirm() is pressed */
    TMap<UClass*, FWidgetCommand> ConfirmRegistry;

    /** Map from widget class to what to do when HandleCancel() is pressed */
    TMap<UClass*, FWidgetCommand> CancelRegistry;
};
