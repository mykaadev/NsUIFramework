#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "NsUIFrameworkTypes.generated.h"

/** The log category of NsUIFramework */
NSUIFRAMEWORK_API DECLARE_LOG_CATEGORY_EXTERN(LogNsUI, Log, All);

/** The layer of an open widget. A higher layer draws on top and gets input first */
UENUM(BlueprintType)
enum class ENsUILayer : uint8
{
    /** HUD parts and widgets of play */
    Game,
    /** Menus such as pause or settings */
    Menu,
    /** Popups such as confirmations */
    Modal
};

/** How an open widget shares input with the game */
UENUM(BlueprintType)
enum class ENsUIInputPolicy : uint8
{
    /** The widget never gets input. Use it for HUD parts, toasts, and prompts */
    Passive,
    /** The widget takes all input. The game gets none */
    UIOnly,
    /** The widget takes the UI actions. All other keys and the sticks go to the game */
    UIAndGame,
    /** The game keeps the focus. The widget handlers get Confirm and Back, and the game also gets the keys */
    GameFirst
};

/** Where the focus goes in an open widget */
UENUM(BlueprintType)
enum class ENsUIFocusPolicy : uint8
{
    /** A child of the widget, such as a button, has the focus */
    Children,
    /** The widget itself has the focus. No child takes it. The handlers get all UI actions */
    Self
};

/** The last kind of device that the player used in the UI */
UENUM(BlueprintType)
enum class ENsUIPointerMode : uint8
{
    /** The mouse. The mouse cursor shows */
    Mouse,
    /** The keyboard or a gamepad. The selection cursor shows */
    Keys
};

/** A UI action. Each one has an Enhanced Input action in the NsUI settings */
UENUM(BlueprintType)
enum class ENsUIAction : uint8
{
    /** Uses the focused control */
    Confirm,
    /** Closes the top widget or popup */
    Back,
    /** Moves the selection up */
    Up,
    /** Moves the selection down */
    Down,
    /** Moves the selection left */
    Left,
    /** Moves the selection right */
    Right,
    /** Goes to the next tab */
    NextTab,
    /** Goes to the previous tab */
    PreviousTab
};

/** How an open widget behaves. The widget gives it in GetConfig */
USTRUCT(BlueprintType)
struct NSUIFRAMEWORK_API FNsUIWidgetConfig
{
    GENERATED_BODY()

    /** Constructor */
    FNsUIWidgetConfig();

    /** The layer. OpenWidget sets it from its Layer argument */
    UPROPERTY(BlueprintReadWrite, Category = "NsUI")
    ENsUILayer Layer;

    /** How the widget shares input with the game */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    ENsUIInputPolicy InputPolicy;

    /** Where the focus goes in the widget */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    ENsUIFocusPolicy FocusPolicy;

    /** A selection mode for world points or actors. None selects the controls of the widget with the focus */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    TSubclassOf<class UNsUISelectionMode> SelectionModeClass;

    /** True shows the mouse cursor while the player uses the mouse */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    bool bShowMouseCursor;
};

/** The first focus of an open widget. The widget gives it in GetFocus */
USTRUCT(BlueprintType)
struct NSUIFRAMEWORK_API FNsUIFocusRequest
{
    GENERATED_BODY()

    /** The control to focus when the widget has no earlier focus. None focuses the first control */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    TObjectPtr<class UWidget> PreferredTarget;
};

/** One world point for the location selection mode */
USTRUCT(BlueprintType)
struct NSUIFRAMEWORK_API FNsUILocation
{
    GENERATED_BODY()

    /** The unique id of the point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    FName Id;

    /** The world position of the point */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    FVector Position = FVector::ZeroVector;
};
