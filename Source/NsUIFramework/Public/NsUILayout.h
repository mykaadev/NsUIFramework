// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUILayout.generated.h"

class UNsUILayer;
class UNsUIWidget;

/**
 * Root layout that owns named layers.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUILayout : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;

    /** Register a layer with a friendly name. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void RegisterLayer(FName LayerName, UNsUILayer* Layer);

    /** Remove a registered layer. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    bool UnregisterLayer(FName LayerName);

    /** Retrieve a layer by name. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    UNsUILayer* GetLayer(FName LayerName) const;

    /** Access to registered layers for internal queries. */
    const TMap<FName, TObjectPtr<UNsUILayer>>& GetRegisteredLayers() const { return RegisteredLayers; }

    /** Clear all stacks in all layers. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void ClearAllLayers() const;

private:
    void EnsureDefaultLayers();

private:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UNsUILayer> GameStaticLayer;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UNsUILayer> GameMenuLayer;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UNsUILayer> GameModalLayer;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UNsUILayer> ApplicationStaticLayer;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UNsUILayer> ApplicationMenuLayer;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UNsUILayer> ApplicationModalLayer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NsUI", meta = (AllowPrivateAccess = "true"))
    TMap<FName, TObjectPtr<UNsUILayer>> RegisteredLayers;
};
