// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "NsUILibrary.h"
#include "NsUISubsystem.generated.h"

class UNsUIWidget;
class UNsUILayer;
class UNsUILayout;
class UNsUICursor;

/**
 * Central coordinator for the Ns UI framework. Relies on engine focus/navigation
 * and provides push/pop, cursor updates, and spatial fallbacks.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUISubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
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

    /** Subsystem singleton helper. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    static UNsUISubsystem* GetSubsystem();

    /** Activate a layout widget, replacing any existing layout. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void SetActiveUILayout(const TSoftClassPtr<UNsUILayout>& InLayoutClass);

    UFUNCTION(BlueprintCallable, Category = "NsUI")
    UNsUILayout* GetActiveUILayout() const { return ActiveUILayout; }

    /** Push a widget class onto the specified layer. Returns the created instance. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    UNsUIWidget* PushWidget(TSubclassOf<UNsUIWidget> WidgetClass, ENsUILayer Layer);

    /** Push an existing widget instance onto the specified layer. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    bool PushWidgetInstance(UNsUIWidget* Widget, ENsUILayer Layer);

    /** Pop the top widget from the specified layer. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void PopWidget(ENsUILayer Layer);

    /** Pop all widgets from the specified layer back to its root. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void PopToRoot(ENsUILayer Layer);

    /** Navigation entry point the game can call from Enhanced Input. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Input")
    void Navigate(EUINavigation Direction);

    /** Confirm action entry point. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Input")
    void Confirm();

    /** Cancel action entry point. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Input")
    void Cancel();

    /** Begin actor selection mode with a set of focusable actors. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Actors")
    void EnterActorSelectionMode(const TArray<AActor*>& Actors);

    /** Exit actor selection mode, restoring widget focus. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Actors")
    void ExitActorSelectionMode();

    /** Called by widgets when they gain focus. */
    void NotifyWidgetFocused(UWidget* Widget);

    /** Called by widgets when they are removed from the focus path. */
    void NotifyWidgetFocusLost(UWidget* Widget);

    /** Current focused widget tracked by the subsystem. */
    UWidget* GetCurrentFocusedWidget() const { return CurrentFocusedWidget.Get(); }

    /** Event fired when an actor is confirmed. */
    FNsUIActorDelegate OnActorConfirmed;

    /** Event fired when an actor is cancelled. */
    FNsUIActorDelegate OnActorCancelled;

    /** Event fired when an actor is toggled. */
    FNsUIActorDelegate OnActorToggled;

private:
    void SetupNavigationConfig();
    UNsUILayer* FindLayer(ENsUILayer Layer) const;
    UNsUILayer* FindLayerForWidget(const UWidget* Widget) const;
    void MoveFocusToWidget(UWidget* TargetWidget);
    UWidget* ResolveEngineNavigation(UWidget* FromWidget, EUINavigation Direction) const;
    UWidget* RunWidgetSpatialFallback(UWidget* FromWidget, EUINavigation Direction) const;
    int32 FindBestActorBySpatialSearch(int32 CurrentIndex, EUINavigation NavDir) const;
    FVector2D GetActorScreenCenter(const FNsUIActorFocusInfo& Info) const;
    void UpdateActorCursor();

private:
    UPROPERTY()
    TObjectPtr<UNsUILayout> ActiveUILayout;

    UPROPERTY()
    TObjectPtr<UNsUICursor> CursorWidget;

    TWeakObjectPtr<UWidget> CurrentFocusedWidget;

    TWeakObjectPtr<UWidget> FocusBeforeActorMode;
    TArray<FNsUIActorFocusInfo> ActorFocusTargets;
    int32 CurrentActorIndex;
    bool bActorSelectionActive;
    bool bCursorEnabled;
};
