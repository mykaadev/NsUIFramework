#include "NsUIWidgetStack.h"

#include "Blueprint/UserWidget.h"
#include "Selection/NsUISelectionMode.h"

FNsUIWidgetEntry& FNsUIWidgetStack::Add(UUserWidget* const Widget, UObject* const Context, const FNsUIWidgetConfig& Config)
{
    FNsUIWidgetEntry& Entry = Entries.AddDefaulted_GetRef();
    Entry.Widget = Widget;
    Entry.Context = Context;
    Entry.Config = Config;
    Entry.Order = NextOrder++;
    return Entry;
}

bool FNsUIWidgetStack::Remove(const UUserWidget* const Widget)
{
    return Entries.RemoveAll([Widget](const FNsUIWidgetEntry& Entry) { return Entry.Widget == Widget; }) > 0;
}

FNsUIWidgetEntry* FNsUIWidgetStack::Find(const UUserWidget* const Widget)
{
    return Entries.FindByPredicate([Widget](const FNsUIWidgetEntry& Entry) { return Entry.Widget == Widget; });
}

const FNsUIWidgetEntry* FNsUIWidgetStack::Find(const UUserWidget* const Widget) const
{
    return Entries.FindByPredicate([Widget](const FNsUIWidgetEntry& Entry) { return Entry.Widget == Widget; });
}

const FNsUIWidgetEntry* FNsUIWidgetStack::FindNewestOfClass(const UClass* const Class) const
{
    for (int32 Index = Entries.Num() - 1; Index >= 0; --Index)
    {
        if (Class != nullptr && Entries[Index].Widget != nullptr && Entries[Index].Widget->IsA(Class) && !Entries[Index].bClosing)
        {
            return &Entries[Index];
        }
    }
    return nullptr;
}

FNsUIWidgetEntry* FNsUIWidgetStack::GetActive()
{
    return Active != nullptr ? Find(Active) : nullptr;
}

const FNsUIWidgetEntry* FNsUIWidgetStack::GetActive() const
{
    return Active != nullptr ? Find(Active) : nullptr;
}

bool FNsUIWidgetStack::UpdateActive(UUserWidget*& OutPrevious)
{
    const FNsUIWidgetEntry* Best = nullptr;
    for (const FNsUIWidgetEntry& Entry : Entries)
    {
        if (TakesInput(Entry) && (Best == nullptr || Entry.Config.Layer > Best->Config.Layer || (Entry.Config.Layer == Best->Config.Layer && Entry.Order > Best->Order)))
        {
            Best = &Entry;
        }
    }

    UUserWidget* const Next = Best != nullptr ? Best->Widget.Get() : nullptr;
    OutPrevious = Find(Active) != nullptr ? Active.Get() : nullptr;
    if (Next == Active)
    {
        return false;
    }
    Active = Next;
    return true;
}

TArray<UUserWidget*> FNsUIWidgetStack::GetWidgetsOnLayer(const ENsUILayer Layer) const
{
    TArray<UUserWidget*> Result;
    for (int32 Index = Entries.Num() - 1; Index >= 0; --Index)
    {
        if (Entries[Index].Config.Layer == Layer)
        {
            Result.Add(Entries[Index].Widget);
        }
    }
    return Result;
}

TArray<UUserWidget*> FNsUIWidgetStack::Clear()
{
    TArray<UUserWidget*> Result;
    for (int32 Index = Entries.Num() - 1; Index >= 0; --Index)
    {
        Result.Add(Entries[Index].Widget);
    }
    Entries.Reset();
    Active = nullptr;
    return Result;
}

void FNsUIWidgetStack::AddReferencedObjects(FReferenceCollector& Collector)
{
    for (FNsUIWidgetEntry& Entry : Entries)
    {
        Collector.AddReferencedObject(Entry.Widget);
        Collector.AddReferencedObject(Entry.Context);
        Collector.AddReferencedObject(Entry.Mode);
    }
    Collector.AddReferencedObject(Active);
}

bool FNsUIWidgetStack::TakesInput(const FNsUIWidgetEntry& Entry)
{
    return Entry.Widget != nullptr && !Entry.bClosing && Entry.Config.InputPolicy != ENsUIInputPolicy::Passive;
}
