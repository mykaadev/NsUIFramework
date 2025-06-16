// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUILayer.generated.h"

/**
 * Night Shift UI Layer
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
    class UUserWidget* Push(const TSoftClassPtr<class UUserWidget>& InClass);

    /** Pop content */
    UFUNCTION(BlueprintCallable)
    class UUserWidget* Pop();

    /** Clear stack */
    UFUNCTION(BlueprintCallable)
    void ClearStack();

    /** Peek */
    UFUNCTION(BlueprintCallable)
    class UUserWidget* Peek();

    /** Collapse top widget */
    UFUNCTION(BlueprintCallable)
    void CollapseTop();

    /** Show top widget*/
    UFUNCTION(BlueprintCallable)
    void ShowTop();

    /** Get stack list names */
    UFUNCTION(BlueprintCallable)
    FText GetStackListNames();

// Variables
public:

    /** Border that contains our widget */
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TObjectPtr<class UBorder> RootBorder;

protected:

    /** Widget Stack */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<class UUserWidget*> WidgetStack;
};
