#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "NsUIFrameworkTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Templates/PimplPtr.h"
#include "Templates/SubclassOf.h"
#include "Types/SlateEnums.h"
#include "NsUISubsystem.generated.h"

/** Called when another widget gets input. Null when no widget takes input */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNsUIActiveWidgetChanged, class UUserWidget*, ActiveWidget);

/** Called when the focus moves inside the active widget */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNsUIFocusChanged, class UUserWidget*, ActiveWidget, class UWidget*, OldFocus, class UWidget*, NewFocus);

/** Called when the player changes between the mouse and the keys or a gamepad */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNsUIPointerModeChanged, ENsUIPointerMode, PointerMode);

/** Called when the player changes the kind of device in the UI. Gamepad is true for a gamepad, false for the mouse or the keyboard */
DECLARE_MULTICAST_DELEGATE_OneParam(FNsUIDeviceUsed, bool /*bGamepad*/);

/**
 * Opens and closes the UI widgets of one local player, and decides which widget gets input
 * It is the only writer of the open widgets, the input mode, the mouse cursor, and the UI navigation keys of its player
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUISubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

// Functions
public:
    /** Constructor */
    UNsUISubsystem();

    //~ Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface

    //~ Begin ULocalPlayerSubsystem Interface
    virtual void PlayerControllerChanged(class APlayerController* NewPlayerController) override;
    //~ End ULocalPlayerSubsystem Interface

    //~ Begin UObject Interface
    static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
    //~ End UObject Interface

    /** Makes a widget of a class and opens it on a layer. Returns null when the player has no player controller yet */
    UFUNCTION(BlueprintCallable, Category = "NsUI", meta = (DeterminesOutputType = "WidgetClass"))
    class UUserWidget* OpenWidget(TSubclassOf<class UUserWidget> WidgetClass, UObject* Context = nullptr, ENsUILayer Layer = ENsUILayer::Menu);

    /** Closes an open widget. A widget whose BeginClose returns true stays until it calls FinishCloseWidget */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void CloseWidget(class UUserWidget* Widget);

    /** Ends the close of a widget after its outro */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void FinishCloseWidget(class UUserWidget* Widget);

    /**
     * Adds a widget that another system made and shows, for example with Add to Viewport. The framework does not move it and does not touch its Slate
     * It routes Confirm and Back to the widget, so the widget config must be GameFirst or Passive. Call RemoveExternalWidget before the widget goes away
     */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void AddExternalWidget(class UUserWidget* Widget, ENsUILayer Layer = ENsUILayer::Game);

    /** Removes a widget that AddExternalWidget added. Returns false when the widget was not an external widget */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    bool RemoveExternalWidget(class UUserWidget* Widget);

    /** Returns true when AddExternalWidget added the widget */
    UFUNCTION(BlueprintPure, Category = "NsUI")
    bool IsExternalWidget(const class UUserWidget* Widget) const;

    /** Closes every widget on a layer */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void CloseAll(ENsUILayer Layer);

    /** Closes the newest open widget of a class, or opens a new one. Returns the new widget, or null when it closed one */
    UFUNCTION(BlueprintCallable, Category = "NsUI", meta = (DeterminesOutputType = "WidgetClass"))
    class UUserWidget* ToggleWidget(TSubclassOf<class UUserWidget> WidgetClass, UObject* Context = nullptr, ENsUILayer Layer = ENsUILayer::Menu);

    /** Returns the newest open widget of a class, or null */
    UFUNCTION(BlueprintPure, Category = "NsUI", meta = (DeterminesOutputType = "WidgetClass"))
    class UUserWidget* FindWidget(TSubclassOf<class UUserWidget> WidgetClass) const;

    /** Returns true while a widget is open and not in its outro */
    UFUNCTION(BlueprintPure, Category = "NsUI")
    bool IsWidgetOpen(const class UUserWidget* Widget) const;

    /** Returns the widget that gets input, or null */
    UFUNCTION(BlueprintPure, Category = "NsUI")
    class UUserWidget* GetActiveWidget() const;

    /** Returns the selection mode of an open widget, or null */
    UFUNCTION(BlueprintPure, Category = "NsUI")
    class UNsUISelectionMode* GetSelectionMode(const class UUserWidget* Widget) const;

    /** Reads the config of an open widget again. Call it when the widget changes its input policy or focus policy */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void RefreshConfig(class UUserWidget* Widget);

    /** Gives the focus to a child of the active widget, for example after the widget rebuilt its list */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void FocusChild(class UUserWidget* Widget, class UWidget* Child);

    /** Gives the focus back to the last focused child of the active widget, or to its first focus */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void RestoreFocus(class UUserWidget* Widget);

    /** Returns the last kind of device that the player used in the UI */
    UFUNCTION(BlueprintPure, Category = "NsUI")
    ENsUIPointerMode GetPointerMode() const { return PointerMode; }

    /** Sets the kind of device that the player uses. The UI root and the optional NsInput module call it */
    void SetPointerMode(ENsUIPointerMode NewPointerMode);

    /** Returns the keys that the player mapped to a UI action */
    UFUNCTION(BlueprintPure, Category = "NsUI")
    TArray<FKey> GetKeysForAction(ENsUIAction Action) const;

    /** Returns the open widgets and the state as text, for debug */
    FString DescribeState() const;

private:
    /** Makes the Slate root and adds it to the viewport of the player */
    void EnsureRoot();

    /** Takes the Slate root out of the viewport */
    void RemoveRoot();

    /** Makes the selection cursor when the settings name one */
    void EnsureCursor();

    /** Takes an open widget out of the stack and the root, and tells it that it closed */
    void RemoveEntry(class UUserWidget* Widget);

    /** Picks the active widget again and applies its input. Force true applies the input also when the active widget is the same */
    void UpdateActive(bool bForce);

    /** Sets the input mode and the mouse cursor of the player controller from the active widget */
    void ApplyInputPolicy();

    /** Shows the selection cursor on the selection of the active widget, or hides it */
    void UpdateCursor();

    /** Binds Confirm and Back for a GameFirst widget, or removes the binding */
    void UpdateGameFirstInput(bool bBind);

    /** Puts the focus where the active widget wants it */
    void ApplyFocus();

    /** Routes a UI action from the root. Preview is true before the focused control gets the key */
    bool RouteAction(ENsUIAction Action, bool bPreview, bool bGamepad);

    /** Routes a direction from the root */
    bool RouteNavigate(EUINavigation Direction);

    /** Routes a pointer event from the root */
    bool RoutePointer(const struct FPointerEvent& Event, bool bPressed);

    /** Returns the UI action of a key */
    TOptional<ENsUIAction> FindAction(const FKey& Key) const;

    /** Handles a device that the player used in the UI */
    void HandleDeviceUsed(ENsUIPointerMode Mode, bool bGamepad);

    /** Handles a focus move of the player */
    void HandleFocusMoved(const TSharedPtr<class SWidget>& OldWidget, const TSharedPtr<class SWidget>& NewWidget);

    /** Handles a change of the suspended state */
    void HandleSuspendedChanged(bool bSuspended);

    /** Handles the cleanup of a world */
    void HandleWorldCleanup(class UWorld* World, bool bSessionEnded, bool bCleanupResources);

    /** Reads the UI keys again after Enhanced Input rebuilt its mappings */
    UFUNCTION()
    void HandleMappingsRebuilt();

    /** Confirm of a GameFirst widget */
    void HandleGameFirstConfirm();

    /** Back of a GameFirst widget */
    void HandleGameFirstBack();

    /** Returns the player controller of the player in the current world, or null */
    class APlayerController* GetPlayerController() const;

    /** Returns the Slate user index of the player, or INDEX_NONE */
    int32 GetUserIndex() const;

    /** Returns true when a Slate widget is inside another, or is the same */
    static bool IsInside(const TSharedPtr<class SWidget>& Ancestor, const TSharedPtr<class SWidget>& Widget);

    /** Returns the first visible, enabled control that can take the focus inside a Slate widget, or null */
    static TSharedPtr<class SWidget> FindFirstFocusable(const TSharedRef<class SWidget>& Widget);

    /** Returns true when a Slate widget can take the focus now */
    static bool CanFocus(const TSharedPtr<class SWidget>& Widget);

    /** Returns the UMG widget of a Slate widget inside a user widget, or the nearest user widget, or null */
    static class UWidget* FindUMGWidget(const TSharedPtr<class SWidget>& Widget);

// Variables
public:
    /** Called when another widget gets input */
    UPROPERTY(BlueprintAssignable, Category = "NsUI")
    FNsUIActiveWidgetChanged OnActiveWidgetChanged;

    /** Called when the focus moves inside the active widget */
    UPROPERTY(BlueprintAssignable, Category = "NsUI")
    FNsUIFocusChanged OnFocusChanged;

    /** Called when the player changes between the mouse and the keys or a gamepad */
    UPROPERTY(BlueprintAssignable, Category = "NsUI")
    FNsUIPointerModeChanged OnPointerModeChanged;

    /** Called when the player changes the kind of device in the UI. The optional NsInput module uses it */
    FNsUIDeviceUsed OnDeviceUsed;

private:
    /** The open widgets. Only this subsystem changes them */
    TPimplPtr<struct FNsUIWidgetStack> Stack;

    /** The keys of the UI actions and the navigation config of the player */
    TPimplPtr<class FNsUIInputActions> InputActions;

    /** The focus watch and the suspended state */
    TPimplPtr<class FNsUIFocusTracker> FocusTracker;

    /** The Slate root of the open widgets */
    TSharedPtr<class SNsUIRoot> Root;

    /** The selection cursor, or null */
    UPROPERTY(Transient)
    TObjectPtr<class UNsUICursorWidget> Cursor;

    /** The input component that gives Confirm and Back to a GameFirst widget */
    UPROPERTY(Transient)
    TObjectPtr<class UEnhancedInputComponent> GameFirstInput;

    /** The last kind of device that the player used */
    ENsUIPointerMode PointerMode;

    /** True when the last device that the player used in the UI was a gamepad */
    bool bLastDeviceGamepad;

    /** The binding of the world cleanup event */
    FDelegateHandle WorldCleanupHandle;
};
