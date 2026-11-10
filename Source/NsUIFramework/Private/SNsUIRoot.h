#pragma once

#include "CoreMinimal.h"
#include "NsUIFrameworkTypes.h"
#include "Widgets/SCompoundWidget.h"

/** Asks the owner for the UI action of a key */
DECLARE_DELEGATE_RetVal_OneParam(TOptional<ENsUIAction>, FNsUIFindAction, const FKey&);

/** Gives a UI action to the owner. Preview is true before the focused control gets the key. Returns true when the owner used it */
DECLARE_DELEGATE_RetVal_ThreeParams(bool, FNsUIRouteAction, ENsUIAction, bool /*bPreview*/, bool /*bGamepad*/);

/** Gives a direction to the owner. Returns true when the owner used it */
DECLARE_DELEGATE_RetVal_OneParam(bool, FNsUIRouteNavigate, EUINavigation);

/** Gives a pointer event to the owner. Pressed is true for a button press. Returns true when the owner used it */
DECLARE_DELEGATE_RetVal_TwoParams(bool, FNsUIRoutePointer, const FPointerEvent&, bool /*bPressed*/);

/** Tells the owner which kind of device the player used. Gamepad is true for a gamepad key */
DECLARE_DELEGATE_TwoParams(FNsUIReportDevice, ENsUIPointerMode, bool /*bGamepad*/);

/** The box around one open widget. It owns the hit test of the widget and keeps navigation inside it */
class SNsUIWidgetSlot : public SCompoundWidget
{
// Functions
public:
    SLATE_BEGIN_ARGS(SNsUIWidgetSlot) {}
        /** The open widget */
        SLATE_DEFAULT_SLOT(FArguments, Content)
        /** Gets the directions before the focus moves */
        SLATE_EVENT(FNsUIRouteNavigate, OnNavigate)
    SLATE_END_ARGS()

    /** Constructor */
    void Construct(const FArguments& Args);

    //~ Begin SWidget Interface
    virtual FNavigationReply OnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& NavigationEvent) override;
    //~ End SWidget Interface

// Variables
private:
    /** Gets the directions before the focus moves */
    FNsUIRouteNavigate OnNavigate;
};

/** The Slate root of the open widgets of one local player. It holds one overlay for each layer and gets the input that no control used */
class SNsUIRoot : public SCompoundWidget
{
// Functions
public:
    SLATE_BEGIN_ARGS(SNsUIRoot) {}
        /** The local player of the root */
        SLATE_ARGUMENT(TWeakObjectPtr<class ULocalPlayer>, Player)
        /** Gives the UI action of a key */
        SLATE_EVENT(FNsUIFindAction, OnFindAction)
        /** Gets the UI actions */
        SLATE_EVENT(FNsUIRouteAction, OnAction)
        /** Gets the directions */
        SLATE_EVENT(FNsUIRouteNavigate, OnNavigate)
        /** Gets the pointer events */
        SLATE_EVENT(FNsUIRoutePointer, OnPointer)
        /** Gets the kind of device that the player used */
        SLATE_EVENT(FNsUIReportDevice, OnDevice)
    SLATE_END_ARGS()

    /** Constructor */
    void Construct(const FArguments& Args);

    //~ Begin SWidget Interface
    virtual bool SupportsKeyboardFocus() const override { return bTakesFocus; }
    virtual FReply OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;
    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;
    virtual FNavigationReply OnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& NavigationEvent) override;
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnPreviewMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    //~ End SWidget Interface

    /** Puts a widget in a new slot on top of its layer and returns the slot */
    TSharedRef<SNsUIWidgetSlot> AddWidget(ENsUILayer Layer, const TSharedRef<SWidget>& Content);

    /** Takes a slot out of its layer */
    void RemoveWidget(const TSharedRef<SWidget>& Slot);

    /** Sets whether the controls in a slot can take clicks */
    static void SetSlotInteractive(const TSharedRef<SWidget>& Slot, bool bInteractive);

    /** Sets whether the root itself takes the pointer. True for world selection, so clicks on empty space reach the mode */
    void SetCatchesPointer(bool bCatches);

    /** Sets whether the root itself can have the focus. True only for a widget that keeps the focus on itself, so a click on empty space never moves the focus to the root */
    void SetTakesFocus(bool bInTakesFocus) { bTakesFocus = bInTakesFocus; }

    /** Returns true while the viewport console of the player is open */
    bool IsConsoleOpen() const;

    /** Returns true when a text box has the focus of a user */
    static bool IsTextFocused(int32 UserIndex);

// Variables
private:
    /** The local player of the root */
    TWeakObjectPtr<class ULocalPlayer> Player;

    /** The overlay of each layer, lowest first */
    TArray<TSharedRef<class SOverlay>> Layers;

    /** Gives the UI action of a key */
    FNsUIFindAction OnFindAction;

    /** Gets the UI actions */
    FNsUIRouteAction OnAction;

    /** Gets the directions */
    FNsUIRouteNavigate OnNavigate;

    /** Gets the pointer events */
    FNsUIRoutePointer OnPointer;

    /** Gets the kind of device that the player used */
    FNsUIReportDevice OnDevice;

    /** True while the root itself can have the focus */
    bool bTakesFocus;
};
