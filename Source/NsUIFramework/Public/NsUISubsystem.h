// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "NsUILibrary.h"
#include "NsUISubsystem.generated.h"

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
    class UUserWidget* PushWidgetToLayer(const TSoftClassPtr<class UUserWidget>& InWidget, ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    class UUserWidget* ReplaceTopWidgetOnLayer(const TSoftClassPtr<class UUserWidget>& InWidget, ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "NS UI Subsystem")
    void PopWidgetFromLayer(ENsUILayerType InLayerType);

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

private:
    UPROPERTY()
    TObjectPtr<class UNsUILayout> ActiveUILayout;
};
