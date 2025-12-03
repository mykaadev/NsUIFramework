// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "INsUINavigation.generated.h"

UINTERFACE(BlueprintType)
class NSUIFRAMEWORK_API UNsUINavigation : public UInterface
{
    GENERATED_BODY()
};

/**
 * Thin semantic hooks that widgets can implement to participate in high-level UI behaviors.
 */
class NSUIFRAMEWORK_API INsUINavigation
{
    GENERATED_BODY()

public:
    /** Invoked when the UI confirm action is triggered while this widget is focused. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void HandleUIConfirm();

    /** Invoked when the UI cancel action is triggered while this widget is focused. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void HandleUICancel();

    /** Called when the widget is pushed onto a layer stack and becomes active. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void OnWidgetActivated();

    /** Called when the widget is removed from the stack or deactivated. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void OnWidgetDeactivated();

    /** Optional: return the widget that should receive initial focus when this widget is activated. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    UWidget* GetInitialFocusWidget() const;
};
