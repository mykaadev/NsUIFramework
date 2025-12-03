// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUILibrary.h"
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

    /** Register a layer against its enum slot. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void RegisterLayer(ENsUILayer LayerSlot, UNsUILayer* Layer);

    /** Remove a registered layer. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    bool UnregisterLayer(ENsUILayer LayerSlot);

    /** Retrieve a layer by enum. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    UNsUILayer* GetLayer(ENsUILayer LayerSlot) const;

    /** Access to registered layers for internal queries. */
    const TMap<ENsUILayer, TObjectPtr<UNsUILayer>>& GetRegisteredLayers() const { return RegisteredLayers; }

    /** Clear all stacks in all layers. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void ClearAllLayers() const;

private:
    void EnsureDefaultLayers();

private:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UNsUILayer> MenuLayer;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UNsUILayer> ModalLayer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NsUI", meta = (AllowPrivateAccess = "true"))
    TMap<ENsUILayer, TObjectPtr<UNsUILayer>> RegisteredLayers;
};
