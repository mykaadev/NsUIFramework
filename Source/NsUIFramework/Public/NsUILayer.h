// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUILayer.generated.h"

class UNsUIWidget;

/**
 * A layer hosts a stack of UNsUIWidget instances and manages focus restoration.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUILayer : public UUserWidget
{
    GENERATED_BODY()

public:
    //~ Begin UUserWidget Interface
    virtual TSharedRef<SWidget> RebuildWidget() override;
    //~ End UUserWidget Interface

    /** Push a widget instance on this layer's stack. */
    bool PushWidget(UNsUIWidget* Widget, UWidget* PreviousFocus);

    /** Pop the top widget off this layer. */
    UNsUIWidget* PopWidget();

    /** Pop until only the root widget remains. */
    void PopToRoot();

    /** Returns the widget currently on top of the stack. */
    UNsUIWidget* PeekWidget() const;

    /** Attempts to restore focus to the last saved widget for this layer. */
    UWidget* RestoreLastFocus();

    /** Root container owning the layer content. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NsUI")
    TObjectPtr<class UPanelWidget> LayerRoot;

private:
    UWidget* FindInitialFocus(UNsUIWidget* Widget) const;

    TArray<TWeakObjectPtr<UNsUIWidget>> WidgetStack;
    TArray<TWeakObjectPtr<UWidget>> FocusHistory;
};
