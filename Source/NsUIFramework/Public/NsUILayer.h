// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUILibrary.h"
#include "NsUILayer.generated.h"

/**
 * Simple UI Layer that owns a stack of widgets.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUILayer : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    /** Push content */
    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* Push(const TSoftClassPtr<class UUserWidget>& InClass);

    /** Replace the current top widget with a new one */
    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* ReplaceTop(const TSoftClassPtr<class UUserWidget>& InClass);

    /** Pop content */
    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* Pop();

    /** Remove a specific widget from the stack */
    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* RemoveWidgetFromStack(class UUserWidget* InWidget);

    /** Clear stack */
    UFUNCTION(BlueprintCallable)
    void ClearStack();

    /** Peek */
    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* Peek();

    /** Get Stack */
    UFUNCTION(BlueprintCallable)
    TArray<class UNsUIWidgetStackElement*> GetStack() const { return Stack; }

private:
    void HideCurrentTop();
    void ShowCurrentTop();
    UNsUIWidgetStackElement* CreateStackElement(UUserWidget* Widget) const;

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UBorder> LayerRoot;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<class UNsUIWidgetStackElement*> Stack;
};
