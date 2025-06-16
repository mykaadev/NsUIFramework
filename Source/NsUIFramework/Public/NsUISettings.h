// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NsUISettings.generated.h"

/**
 * Night Shift UI Settings
 */
UCLASS(Config=Game, DefaultConfig)
class NSUIFRAMEWORK_API UNsUISettings : public UDeveloperSettings
{
    GENERATED_BODY()

// Functions
public:

    /** Get Game Settings */
    UFUNCTION(BlueprintCallable, Category = "Game Settings")
    static UNsUISettings* GetSettings()
    {
        static UNsUISettings* Instance;
        if(Instance != nullptr)
        {
            return Instance;
        }

        for (const TObjectIterator<UNsUISettings> SettingsIt(RF_NoFlags); SettingsIt;)
        {
            Instance = *SettingsIt;
            break;
        }
        return Instance;
    }

// Variables
public:

    /** UI Layouts: Layout Name - Layout Object*/
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Ns UI Settings")
    TMap<FString, TSoftClassPtr<class UNsUILayout>> UILayouts;
};
