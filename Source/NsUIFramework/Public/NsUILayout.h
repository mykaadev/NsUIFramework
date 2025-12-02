// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUILayer.h"
#include "NsUILayout.generated.h"

/**
 * Simple UI Layout that holds multiple layers.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUILayout : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;

    UFUNCTION(BlueprintCallable)
    void RegisterLayer(ENsUILayerType InLayerType, class UNsUILayer* InLayer);

    UFUNCTION(BlueprintCallable)
    bool UnregisterLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* PushContentToLayer(ENsUILayerType InLayerType, const TSoftClassPtr<class UUserWidget>& InWidgetClass, bool bCollapsePrevious = true);

    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* ReplaceTopWidgetOnLayer(ENsUILayerType InLayerType, const TSoftClassPtr<class UUserWidget>& InWidgetClass);

    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* PopContentFromLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* RemoveWidgetFromLayer(ENsUILayerType InLayerType, class UUserWidget* InWidget);

    UFUNCTION(BlueprintCallable)
    void ClearLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable)
    TArray<class UNsUIWidgetStackElement*> UnwindLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable)
    void ClearAllLayers() const;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> GameStaticLayer;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> GameMenuLayer;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> GameModalLayer;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> ApplicationStaticLayer;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> ApplicationMenuLayer;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> ApplicationModalLayer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TMap<ENsUILayerType, UNsUILayer*> RegisteredLayers;
};
