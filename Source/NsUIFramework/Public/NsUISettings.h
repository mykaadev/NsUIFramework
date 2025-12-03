// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "NsUISettings.generated.h"

/**
 * Nulled Softworks UI Settings
 */
UCLASS(Config=Game, DefaultConfig)
class NSUIFRAMEWORK_API UNsUISettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UNsUISettings(const FObjectInitializer& ObjectInitializer);

    /** Get Game Settings */
    UFUNCTION(BlueprintCallable, Category = "Game Settings")
    static UNsUISettings* GetSettings();

    /** Widget used to visualize which element currently has focus */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cursor")
    TSoftClassPtr<class UNsUICursor> CursorWidgetClass;

    /** Whether the retro cursor should be enabled by default. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Cursor")
    bool bEnableRetroCursor = true;

    /** Enable keyboard navigation through Slate. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Navigation")
    bool bEnableKeyNavigation = true;

    /** Enable tab navigation through Slate. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Navigation")
    bool bEnableTabNavigation = true;

    /** Enable analog navigation through Slate. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Navigation")
    bool bEnableAnalogNavigation = true;

    /** Optional override for analog horizontal key. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Navigation")
    FKey AnalogHorizontalKey;

    /** Optional override for analog vertical key. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Navigation")
    FKey AnalogVerticalKey;
};
