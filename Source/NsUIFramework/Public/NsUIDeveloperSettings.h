#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "NsUIDeveloperSettings.generated.h"

/**
 * Project-configurable input bindings for NsUI navigation and confirmation.
 */
UCLASS(config = Game, defaultconfig, DisplayName = "NsUI Framework")
class NSUIFRAMEWORK_API UNsUIDeveloperSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UNsUIDeveloperSettings();

    /** Keys that trigger confirm/accept. */
    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TArray<FKey> ConfirmKeys;

    /** Keys that trigger cancel/back. */
    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TArray<FKey> CancelKeys;

    /** Keys for directional navigation. */
    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TArray<FKey> NavigateUpKeys;

    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TArray<FKey> NavigateDownKeys;

    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TArray<FKey> NavigateLeftKeys;

    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TArray<FKey> NavigateRightKeys;

    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TArray<FKey> NavigateNextKeys;

    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TArray<FKey> NavigatePreviousKeys;

    /** Mouse buttons that should act as confirm in NsUI. */
    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TArray<FKey> ConfirmMouseButtons;
};
