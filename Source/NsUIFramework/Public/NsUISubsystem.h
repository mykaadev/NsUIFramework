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
 * Minimal UI subsystem that centralizes layer transitions.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUISubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNsUISubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    static UNsUISubsystem* GetSubsystem();

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    class UUserWidget* PushWidgetToLayer(const TSoftClassPtr<class UUserWidget>& InWidget, ENsUILayerType InLayerType, bool bCollapsePrevious = true);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    class UUserWidget* ReplaceTopWidgetOnLayer(const TSoftClassPtr<class UUserWidget>& InWidget, ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void PopWidgetFromLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void UnwindLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void RemoveWidgetFromLayer(ENsUILayerType InLayerType, class UUserWidget* InWidget);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void ClearLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    class UNsUIWidgetStackElement* ApplyLayerTransition(const FNsUILayerTransitionRequest& Request);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void SetActiveUILayout(const TSoftClassPtr<class UNsUILayout>& InLayout);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    UNsUILayout* GetActiveUILayout() const;

    /** Optional retro cursor helpers */
    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void SetRetroCursorClass(const TSoftClassPtr<class UNsUICursor>& InCursorClass);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    class UNsUICursor* EnsureRetroCursor();

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void MoveRetroCursorToWidget(class UWidget* TargetWidget);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void MoveRetroCursorToScreenPosition(const FVector2D& ScreenPosition);

    /** Lightweight actor selection session helpers */
    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void StartActorSelectionSession(const TArray<TSoftObjectPtr<class AActor>>& Actors);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void ExitActorSelectionSession();

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    class AActor* GetCurrentActorSelection() const;

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    class AActor* StepActorSelection(int32 Direction);

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
};
