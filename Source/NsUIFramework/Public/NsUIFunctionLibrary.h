#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsUIFrameworkTypes.h"
#include "Templates/SubclassOf.h"
#include "NsUIFunctionLibrary.generated.h"

/** Blueprint calls that reach the NsUI subsystem of a player controller */
UCLASS()
class NSUIFRAMEWORK_API UNsUIFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

// Functions
public:
    /** Returns the NsUI subsystem of a local player controller, or null */
    UFUNCTION(BlueprintPure, Category = "NsUI")
    static class UNsUISubsystem* GetUISubsystem(const class APlayerController* Player);

    /** Opens a widget for a player. Returns null when the player has no NsUI subsystem */
    UFUNCTION(BlueprintCallable, Category = "NsUI", meta = (DeterminesOutputType = "WidgetClass"))
    static class UUserWidget* OpenWidget(const class APlayerController* Player, TSubclassOf<class UUserWidget> WidgetClass, UObject* Context = nullptr, ENsUILayer Layer = ENsUILayer::Menu);

    /** Closes the newest open widget of a class for a player, or opens a new one */
    UFUNCTION(BlueprintCallable, Category = "NsUI", meta = (DeterminesOutputType = "WidgetClass"))
    static class UUserWidget* ToggleWidget(const class APlayerController* Player, TSubclassOf<class UUserWidget> WidgetClass, UObject* Context = nullptr, ENsUILayer Layer = ENsUILayer::Menu);

    /** Closes an open widget of its own player */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    static void CloseWidget(class UUserWidget* Widget);
};
