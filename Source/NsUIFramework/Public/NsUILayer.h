// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsUILibrary.h"
#include "Blueprint/UserWidget.h"
#include "NsUILayer.generated.h"

/**
 * Nulled Softworks UI Layer
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUILayer : public UUserWidget
{
    GENERATED_BODY()

// Functions
public:

    //~ Begin UUserWidget Interface
    virtual void NativeConstruct() override;
    //~ End UUserWidget Interface

    /** Push content */
    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* Push(const TSoftClassPtr<class UUserWidget>& InClass);

    /** Pop content */
    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* Pop();

    /** Get Stack */
    UFUNCTION(BlueprintCallable)
    TArray<class UNsUIWidgetStackElement*> GetStack() const;

    /** Clear stack */
    UFUNCTION(BlueprintCallable)
    void ClearStack();

    /** Peek */
    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* Peek();

    /** Collapse top widget */
    UFUNCTION(BlueprintCallable)
    void CollapseTop();

    /** Show top widget*/
    UFUNCTION(BlueprintCallable)
    void ShowTop();

    /** Find Stack Element */
    UFUNCTION(BlueprintCallable)
    class UNsUIWidgetStackElement* FindStackElement(const UUserWidget* InInvokerScope, const UWidget* InInvokerElement);

    /** Get stack list names */
    UFUNCTION(BlueprintCallable)
    FText GetStackListNames();

    /** Gets the initial focus for a widget */
    UFUNCTION(BlueprintCallable)
    class UWidget* GetInitialFocusForWidget(class UUserWidget* InWidget);

private:

    /** Handles the push functionality of the widget when it implements our interface */
    UFUNCTION()
    void PushWidget(class UUserWidget* InWidget);

// Variables
public:

    /** Border that contains our widget */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UBorder> LayerRoot;

protected:

    /** Widget Stack */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<class UNsUIWidgetStackElement*> Stack;
};
