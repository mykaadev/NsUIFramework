// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "NsUILibrary.h"
#include "NsUISubsystem.generated.h"

class UNsUICursor;
class UWidget;
class AActor;

/**
 * Minimal UI subsystem that centralizes layouts, layers, the retro cursor, and actor selection.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUISubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    UNsUISubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // FTickableGameObject
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return true; }
    virtual bool IsTickableWhenPaused() const override { return true; }

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    static UNsUISubsystem* GetSubsystem();

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Layers")
    class UUserWidget* PushWidgetToLayer(const TSoftClassPtr<class UUserWidget>& InWidget, ENsUILayerType InLayerType, const FNsUILayerTransitionRequest& Options);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Layers")
    class UUserWidget* ReplaceTopWidgetOnLayer(const TSoftClassPtr<class UUserWidget>& InWidget, ENsUILayerType InLayerType, const FNsUILayerTransitionRequest& Options);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Layers")
    void PopWidgetFromLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Layers")
    void UnwindLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Layers")
    void RemoveWidgetFromLayer(ENsUILayerType InLayerType, class UUserWidget* InWidget);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Layers")
    int32 RemoveWidgetsWithTag(ENsUILayerType InLayerType, FName Tag);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Layers")
    void RemoveWidgetByStableId(ENsUILayerType InLayerType, const FGuid& StableId);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Layers")
    void ClearLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Layers")
    void ClearAllLayers();

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Layers")
    class UNsUIWidgetStackElement* ApplyLayerTransition(const FNsUILayerTransitionRequest& Request);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Layouts")
    void SetActiveUILayout(const TSoftClassPtr<class UNsUILayout>& InLayout);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Layouts")
    UNsUILayout* GetActiveUILayout() const;

    /** Retro cursor helpers */
    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Cursor")
    void SetRetroCursorClass(const TSoftClassPtr<class UNsUICursor>& InCursorClass);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Cursor")
    class UNsUICursor* EnsureRetroCursor();

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Cursor")
    void MoveRetroCursorToWidget(class UWidget* TargetWidget);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Cursor")
    void MoveRetroCursorToScreenPosition(const FVector2D& ScreenPosition);

    /** Lightweight actor selection session helpers */
    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Actor Selection")
    void StartActorSelectionSession(const TArray<TSoftObjectPtr<class AActor>>& Actors, bool bSnapCursorToSelection = true);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Actor Selection")
    void ExitActorSelectionSession();

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Actor Selection")
    class AActor* GetCurrentActorSelection() const;

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Actor Selection")
    class AActor* StepActorSelection(int32 Direction, bool bSnapCursorToSelection = true);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem|Telemetry")
    const TArray<FNsUIStateEvent>& GetRecentStateEvents() const { return RecentStateEvents; }

private:
    void DestroyActiveLayout();
    void UpdateRetroCursorFromActorSelection();
    bool ProjectActorToScreen(const AActor* Actor, FVector2D& OutScreenPos) const;
    void LogStateEvent(const UNsUIWidgetStackElement* Element, ENsUILayerTransitionOp Op);

private:
    UPROPERTY()
    TObjectPtr<class UNsUILayout> ActiveUILayout;

    UPROPERTY()
    TObjectPtr<class UNsUICursor> RetroCursor;

    UPROPERTY()
    TSoftClassPtr<class UNsUICursor> RetroCursorClass;

    UPROPERTY()
    TArray<TWeakObjectPtr<class AActor>> ActorSelectionCandidates;

    UPROPERTY()
    int32 ActorSelectionIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NS UI Subsystem|Actor Selection")
    bool bCursorTracksActorSelection = true;

    UPROPERTY()
    TArray<FNsUIStateEvent> RecentStateEvents;
};
