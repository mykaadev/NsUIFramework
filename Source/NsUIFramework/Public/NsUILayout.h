// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUILayer.h"
#include "NsUILayout.generated.h"

/**
 * UI Layout that owns multiple layers and proxies stack operations to them.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUILayout : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layout")
    void RegisterLayer(ENsUILayerType InLayerType, class UNsUILayer* InLayer);

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layout")
    bool UnregisterLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layout")
    class UNsUIWidgetStackElement* PushContentToLayer(ENsUILayerType InLayerType, const TSoftClassPtr<class UUserWidget>& InWidgetClass, bool bCollapsePrevious = true);

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layout")
    class UNsUIWidgetStackElement* ReplaceTopWidgetOnLayer(ENsUILayerType InLayerType, const TSoftClassPtr<class UUserWidget>& InWidgetClass, bool bCollapsePrevious = true);

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layout")
    class UNsUIWidgetStackElement* PopContentFromLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layout")
    class UNsUIWidgetStackElement* RemoveWidgetFromLayer(ENsUILayerType InLayerType, class UUserWidget* InWidget);

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layout")
    void ClearLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layout")
    TArray<class UNsUIWidgetStackElement*> UnwindLayer(ENsUILayerType InLayerType);

    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layout")
    void ClearAllLayers() const;

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> GameStaticLayer = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> GameMenuLayer = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> GameModalLayer = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> ApplicationStaticLayer = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> ApplicationMenuLayer = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> ApplicationModalLayer = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework|Layout")
    TMap<ENsUILayerType, UNsUILayer*> RegisteredLayers;
};
