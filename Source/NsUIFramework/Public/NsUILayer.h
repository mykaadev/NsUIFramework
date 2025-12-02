// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUILibrary.h"
#include "NsUILayer.generated.h"

class UPanelWidget;
class UBorder;

/**
 * UI Layer that owns a stack of widgets and optionally shows multiple at once when backed by a panel.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUILayer : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    /** Push a widget class onto the layer stack. Optionally collapse the previous top entry. */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layer")
    class UNsUIWidgetStackElement* PushClass(const TSoftClassPtr<class UUserWidget>& InClass, const FNsUILayerTransitionRequest& Options);

    /** Push an existing widget instance. Optionally collapse the previous top entry. */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layer")
    class UNsUIWidgetStackElement* PushWidget(class UUserWidget* InWidget, const FNsUILayerTransitionRequest& Options);

    /** Replace the current top widget with a new class instance. */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layer")
    class UNsUIWidgetStackElement* ReplaceTopWithClass(const TSoftClassPtr<class UUserWidget>& InClass, const FNsUILayerTransitionRequest& Options);

    /** Pop the top widget from the layer. */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layer")
    class UNsUIWidgetStackElement* Pop();

    /** Remove a specific widget from the stack (no blind popping required). */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layer")
    class UNsUIWidgetStackElement* RemoveWidgetFromStack(class UUserWidget* InWidget);

    /** Remove widgets by tag. */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layer")
    int32 RemoveWidgetsWithTag(FName Tag);

    /** Remove by stable identifier. */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layer")
    class UNsUIWidgetStackElement* RemoveByStableId(const FGuid& StableId);

    /** Clear the layer entirely. */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layer")
    void ClearStack();

    /** Pop every widget from the stack and return the removed elements (top-first). */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layer")
    TArray<class UNsUIWidgetStackElement*> Unwind();

    /** Peek the current top element. */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layer")
    class UNsUIWidgetStackElement* Peek() const;

    /** Blueprint-friendly view of the current stack. */
    UFUNCTION(BlueprintCallable, Category = "Ns UI Framework|Layer")
    const TArray<class UNsUIWidgetStackElement*>& GetStack() const { return Stack; }

private:
    UNsUIWidgetStackElement* CreateStackElement(UUserWidget* Widget, const FNsUILayerTransitionRequest& Options) const;
    void AttachToLayer(UUserWidget* Widget) const;
    void DetachFromLayer(UUserWidget* Widget) const;
    void ShowTopAndRestoreFocus(ENsUIFocusRule FocusRule = ENsUIFocusRule::Auto);
    void NotifyWidgetPushed(UUserWidget* Widget) const;
    void CollapsePreviousIfNeeded(bool bCollapsePrevious) const;
    void TrackFocusBeforeTransition();
    void RestorePreviousFocus();

public:
    /** Root slot when showing a single widget at a time. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> LayerRoot = nullptr;

    /** Optional panel root to support multiple visible widgets on a single layer. */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UPanelWidget> LayerPanel = nullptr;

    /** Priority for focus and routing. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ns UI Framework|Layer")
    ENsUILayerPriority LayerPriority = ENsUILayerPriority::Menu;

protected:
    /** Stack of widgets owned by this layer. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ns UI Framework|Layer")
    TArray<class UNsUIWidgetStackElement*> Stack;

    /** Cached focus target before a transition to support deterministic restoration. */
    UPROPERTY(Transient)
    TWeakObjectPtr<class UWidget> PreviousFocusTarget;
};
