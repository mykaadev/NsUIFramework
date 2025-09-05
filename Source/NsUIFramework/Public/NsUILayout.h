// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsUILayer.h"
#include "NsUILibrary.h"
#include "Blueprint/UserWidget.h"
#include "NsUILayout.generated.h"

/**
 * Nulled Softworks UI Layout
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUILayout : public UUserWidget
{
    GENERATED_BODY()

// Functions
public:

    //~ Begin UUserWidget Interface
    virtual void NativeOnInitialized() override;
    //~ End UUserWidget Interface

    /** Register Layer */
    UFUNCTION(BlueprintCallable)
    void RegisterLayer(const ENsUILayerType InLayerType, class UNsUILayer* const InLayer);

    /** Unregister Layer */
    UFUNCTION(BlueprintCallable)
    bool UnregisterLayer(const ENsUILayerType InLayerType);

    /** Push content to layer */
    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* PushContentToLayer(const ENsUILayerType InLayerType, const TSoftClassPtr<class UUserWidget>& InWidgetClass);

    /** Pop content form layer */
    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* PopContentFromLayer(ENsUILayerType InLayerType);

    /** Clear all layers */
    UFUNCTION(BlueprintCallable)
    void ClearAllLayers() const;

    /** Clear layer */
    UFUNCTION(BlueprintCallable)
    void ClearLayer(const ENsUILayerType InLayerType);

    /** Find Invoker widget in this layout */
    class UNsUIWidgetStackElement* FindInvokerWidgetInLayout(const class UNsUIWidgetStackElement* InWidget);

    // Variables
public:

    /** Any static elements present in the game (eg: prompts, time, navigation) */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> GameStaticLayer;

    /** Any pop-up elements present in the game (eg: inventory, shops) */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> GameMenuLayer;

    /** Any pop-up elements present in the game (eg: confirmations) */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> GameModalLayer;

    /** Any static elements present in the application (eg: version, prompts) */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> ApplicationStaticLayer;

    /** Any static elements present in the application (eg: menus, consoles, debug) */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> ApplicationMenuLayer;

    /** Any pop-up elements present in the application (eg: confirmations) */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UNsUILayer> ApplicationModalLayer;

    /** Registered Layers */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TMap<ENsUILayerType, UNsUILayer*> RegisteredLayers;
};
