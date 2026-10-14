#pragma once

#include "NsUIFrameworkTypes.h"
#include "UObject/Interface.h"
#include "NsUIScreen.generated.h"

UINTERFACE(BlueprintType)
class NSUIFRAMEWORK_API UNsUIScreen : public UInterface
{
    GENERATED_BODY()
};

/** Optional integration for hosted and viewport screens */
class NSUIFRAMEWORK_API INsUIScreen
{
    GENERATED_BODY()

public:
    /** Supply input policy and the selection mode class */
    UFUNCTION(BlueprintNativeEvent, Category="NsUI")
    FNsUIScreenConfig GetNsScreenConfig() const;

    /** Supply a preferred target or request logical input */
    UFUNCTION(BlueprintNativeEvent, Category="NsUI")
    FNsUISelectionRequest GetNsScreenSelection() const;

    /** Receive context and configure the mode instance */
    UFUNCTION(BlueprintNativeEvent, Category="NsUI")
    void OnNsScreenOpened(UObject* Context, UNsUISelectionMode* SelectionMode);

    /** Receive activation; bResumed identifies a restored screen */
    UFUNCTION(BlueprintNativeEvent, Category="NsUI")
    void OnNsScreenActivated(bool bResumed);

    /** Release interaction while retaining screen and mode state */
    UFUNCTION(BlueprintNativeEvent, Category="NsUI")
    void OnNsScreenSuspended();

    /** Release screen resources when its registration ends */
    UFUNCTION(BlueprintNativeEvent, Category="NsUI")
    void OnNsScreenClosed();

    /** Return true to consume Confirm before native control input */
    UFUNCTION(BlueprintNativeEvent, Category="NsUI")
    bool HandleNsScreenConfirm();

    /** Return true to consume Cancel before the host default */
    UFUNCTION(BlueprintNativeEvent, Category="NsUI")
    bool HandleNsScreenCancel();
};
