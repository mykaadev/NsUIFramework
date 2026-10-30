#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsUIFrameworkTypes.h"
#include "Templates/SubclassOf.h"
#include "NsUIFunctionLibrary.generated.h"

class APlayerController;
class UNsUISubsystem;
class UUserWidget;

/** Blueprint shortcuts to the screen host of one player */
UCLASS()
class NSUIFRAMEWORK_API UNsUIFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Return the screen host of a local player controller, or null */
    UFUNCTION(BlueprintPure, Category="NsUI")
    static UNsUISubsystem* GetUISubsystem(const APlayerController* Player);

    /** Open a screen for a local player on a layer */
    UFUNCTION(BlueprintCallable, Category="NsUI", meta=(DeterminesOutputType="ScreenClass", AdvancedDisplay="Context"))
    static UUserWidget* OpenScreen(APlayerController* Player, TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer = ENsUILayer::Menu, UObject* Context = nullptr);

    /** Close the screen when it is open, otherwise open it. Return the open screen, or null after a close */
    UFUNCTION(BlueprintCallable, Category="NsUI", meta=(DeterminesOutputType="ScreenClass"))
    static UUserWidget* ToggleScreen(APlayerController* Player, TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer = ENsUILayer::Menu);

    /** Close a screen that the host opened. A screen can close itself with Self */
    UFUNCTION(BlueprintCallable, Category="NsUI")
    static void CloseScreen(UUserWidget* Screen);
};
