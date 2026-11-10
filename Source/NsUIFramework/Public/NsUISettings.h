#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NsUIFrameworkTypes.h"
#include "NsUISettings.generated.h"

/** The project settings of NsUIFramework. Project Settings > Plugins > Ns UI Framework */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Ns UI Framework"))
class NSUIFRAMEWORK_API UNsUISettings : public UDeveloperSettings
{
    GENERATED_BODY()

// Functions
public:
    /** Constructor */
    UNsUISettings();

    /** Returns the Enhanced Input action of a UI action, or null */
    const class UInputAction* GetInputAction(ENsUIAction Action) const;

// Variables
public:
    /** The mapping context with the keys of the UI actions. The plugin has a default one: IMC_NsUI */
    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TSoftObjectPtr<class UInputMappingContext> InputMappingContext;

    /** The priority of the UI mapping context. The UI actions do not consume input, so the priority changes nothing for the game */
    UPROPERTY(Config, EditAnywhere, Category = "Input")
    int32 InputMappingPriority;

    /** The Enhanced Input action of each UI action. The plugin has defaults: IA_NsUI_Confirm and the others */
    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TMap<ENsUIAction, TSoftObjectPtr<class UInputAction>> InputActions;

    /** The widget that shows the selection while the player uses keys or a gamepad. None shows no selection cursor */
    UPROPERTY(Config, EditAnywhere, Category = "Presentation")
    TSoftClassPtr<class UNsUICursorWidget> CursorWidgetClass;

    /** The viewport order of the widget root. A higher value draws over other viewport widgets */
    UPROPERTY(Config, EditAnywhere, Category = "Presentation")
    int32 RootZOrder;

    /** The viewport order of the selection cursor */
    UPROPERTY(Config, EditAnywhere, Category = "Presentation")
    int32 CursorZOrder;

    /** The distance in viewport units in which the mouse selects a world candidate */
    UPROPERTY(Config, EditAnywhere, Category = "World Selection", meta = (ClampMin = "0"))
    float PointerPickRadius;

    /** The widest angle from a direction in which a world candidate can be the next selection */
    UPROPERTY(Config, EditAnywhere, Category = "World Selection", meta = (ClampMin = "1", ClampMax = "90", Units = "deg"))
    float WorldNavigationConeDegrees;
};
