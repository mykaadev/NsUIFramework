#pragma once

#include "CoreMinimal.h"
#include "NsUIFrameworkTypes.h"
#include "UObject/ObjectPtr.h"

/** One open widget. Only FNsUIWidgetStack makes and changes entries */
struct FNsUIWidgetEntry
{
    /** The open widget */
    TObjectPtr<class UUserWidget> Widget;

    /** The object given to OpenWidget. The entry keeps it alive while the widget is open */
    TObjectPtr<UObject> Context;

    /** The selection mode of the widget, or null */
    TObjectPtr<class UNsUISelectionMode> Mode;

    /** The config that the widget gave at open or at the last RefreshConfig */
    FNsUIWidgetConfig Config;

    /** The Slate box around the widget in the root. It owns the hit test and the navigation boundary */
    TSharedPtr<class SWidget> Slot;

    /** The last child with the focus, for focus restore */
    TWeakPtr<class SWidget> LastFocus;

    /** The open order. A newer widget has a higher value */
    int32 Order = 0;

    /** True after the widget had input one time */
    bool bWasActive = false;

    /** True while the widget plays its outro. A closing widget gets no input */
    bool bClosing = false;
};

/** The open widgets of one local player, and the rule that picks the active one. Plain C++, so tests can drive it without Slate */
struct FNsUIWidgetStack
{
// Functions
public:
    /** Adds an entry and returns it */
    FNsUIWidgetEntry& Add(class UUserWidget* Widget, UObject* Context, const FNsUIWidgetConfig& Config);

    /** Removes the entry of a widget. Returns false when the widget is not open */
    bool Remove(const class UUserWidget* Widget);

    /** Returns the entry of a widget, or null */
    FNsUIWidgetEntry* Find(const class UUserWidget* Widget);

    /** Returns the entry of a widget, or null */
    const FNsUIWidgetEntry* Find(const class UUserWidget* Widget) const;

    /** Returns the newest entry whose widget is of a class, or null */
    const FNsUIWidgetEntry* FindNewestOfClass(const UClass* Class) const;

    /** Returns the active entry, or null */
    FNsUIWidgetEntry* GetActive();

    /** Returns the active entry, or null */
    const FNsUIWidgetEntry* GetActive() const;

    /** Picks the active entry again: the highest layer that takes input, then the newest. Returns true when it changed and gives the earlier active widget */
    bool UpdateActive(class UUserWidget*& OutPrevious);

    /** Returns the widgets of a layer, newest first */
    TArray<class UUserWidget*> GetWidgetsOnLayer(ENsUILayer Layer) const;

    /** Returns all entries, oldest first */
    const TArray<FNsUIWidgetEntry>& GetEntries() const { return Entries; }

    /** Removes every entry and returns the widgets, newest first */
    TArray<class UUserWidget*> Clear();

    /** Reports the objects of the entries to the garbage collector */
    void AddReferencedObjects(FReferenceCollector& Collector);

    /** Returns true when an entry takes input */
    static bool TakesInput(const FNsUIWidgetEntry& Entry);

// Variables
private:
    /** The open widgets, oldest first */
    TArray<FNsUIWidgetEntry> Entries;

    /** The widget of the active entry */
    TObjectPtr<class UUserWidget> Active;

    /** The order of the next entry */
    int32 NextOrder = 0;
};
