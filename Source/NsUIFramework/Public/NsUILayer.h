// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUILayer.generated.h"

class UUserWidget;
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
    bool PushWidget(UUserWidget* Widget, UWidget* PreviousFocus);

    /** Pop the top widget off this layer. */
    UUserWidget* PopWidget();

    /** Pop until only the root widget remains. */
    void PopToRoot();

    /** Returns the widget currently on top of the stack. */
    UUserWidget* PeekWidget() const;

    /** Attempts to restore focus to the last saved widget for this layer. */
    UWidget* RestoreLastFocus();

    /** Root container owning the layer content. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NsUI")
    TObjectPtr<class UPanelWidget> LayerRoot;

private:
    UWidget* FindInitialFocus(UUserWidget* Widget) const;

    TArray<TWeakObjectPtr<UUserWidget>> WidgetStack;
    TArray<TWeakObjectPtr<UWidget>> FocusHistory;
};
