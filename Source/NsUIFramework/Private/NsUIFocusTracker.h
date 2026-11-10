#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "UObject/WeakObjectPtr.h"

/** Watches the Slate focus of one local player. It owns the suspended state and the timing of focus restore. The subsystem decides where the focus goes */
class FNsUIFocusTracker
{
// Functions
public:
    /** Called when the focus of the player moves. Gives the old and the new focused widget */
    DECLARE_DELEGATE_TwoParams(FOnFocusMoved, const TSharedPtr<class SWidget>&, const TSharedPtr<class SWidget>&);

    /** Called when the suspended state changes */
    DECLARE_DELEGATE_OneParam(FOnSuspendedChanged, bool);

    /** Called on the frame after RequestRestore */
    DECLARE_DELEGATE(FOnRestore);

    /** Constructor */
    FNsUIFocusTracker(class ULocalPlayer* InPlayer, FOnFocusMoved InOnFocusMoved, FOnSuspendedChanged InOnSuspendedChanged, FOnRestore InOnRestore);

    /** Removes the Slate bindings */
    ~FNsUIFocusTracker();

    /** Returns true while the game viewport of the player has no focus, for example while the player uses another window */
    bool IsSuspended() const { return bSuspended; }

    /** Asks for one focus restore on the next frame. More requests in the same frame make one restore */
    void RequestRestore();

    /** Returns the Slate user index of the player, or INDEX_NONE */
    int32 GetUserIndex() const;

private:
    /** Handles a Slate focus change of any user */
    void HandleFocusChanging(const struct FFocusEvent& Event, const class FWeakWidgetPath& OldPath, const TSharedPtr<class SWidget>& OldWidget, const class FWidgetPath& NewPath, const TSharedPtr<class SWidget>& NewWidget);

    /** Handles the activation of the application */
    void HandleActivationChanged(bool bActive);

    /** Runs the restore that RequestRestore asked for */
    bool HandleRestoreTick(float DeltaSeconds);

    /** Sets the suspended state and reports a change */
    void SetSuspended(bool bInSuspended);

    /** Returns true when a widget is in the window of the game viewport of the player */
    bool IsInGameWindow(const TSharedPtr<class SWidget>& Widget) const;

// Variables
private:
    /** The local player of the focus */
    TWeakObjectPtr<class ULocalPlayer> Player;

    /** Called when the focus moves */
    FOnFocusMoved OnFocusMoved;

    /** Called when the suspended state changes */
    FOnSuspendedChanged OnSuspendedChanged;

    /** Called for a focus restore */
    FOnRestore OnRestore;

    /** The binding of the Slate focus event */
    FDelegateHandle FocusHandle;

    /** The binding of the Slate activation event */
    FDelegateHandle ActivationHandle;

    /** The pending restore on the core ticker */
    FTSTicker::FDelegateHandle RestoreHandle;

    /** True while the game viewport of the player has no focus */
    bool bSuspended;
};
