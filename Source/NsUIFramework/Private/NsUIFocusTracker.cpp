#include "NsUIFocusTracker.h"

#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Widgets/SViewport.h"
#include "Widgets/SWindow.h"

FNsUIFocusTracker::FNsUIFocusTracker(ULocalPlayer* const InPlayer, FOnFocusMoved InOnFocusMoved, FOnSuspendedChanged InOnSuspendedChanged, FOnRestore InOnRestore)
    : Player(InPlayer)
    , OnFocusMoved(MoveTemp(InOnFocusMoved))
    , OnSuspendedChanged(MoveTemp(InOnSuspendedChanged))
    , OnRestore(MoveTemp(InOnRestore))
    , bSuspended(false)
{
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication& Slate = FSlateApplication::Get();
        FocusHandle = Slate.OnFocusChanging().AddRaw(this, &FNsUIFocusTracker::HandleFocusChanging);
        ActivationHandle = Slate.OnApplicationActivationStateChanged().AddRaw(this, &FNsUIFocusTracker::HandleActivationChanged);
    }
}

FNsUIFocusTracker::~FNsUIFocusTracker()
{
    FTSTicker::GetCoreTicker().RemoveTicker(RestoreHandle);
    FTSTicker::GetCoreTicker().RemoveTicker(MovedHandle);
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication& Slate = FSlateApplication::Get();
        Slate.OnFocusChanging().Remove(FocusHandle);
        Slate.OnApplicationActivationStateChanged().Remove(ActivationHandle);
    }
}

void FNsUIFocusTracker::RequestRestore()
{
    if (!RestoreHandle.IsValid())
    {
        RestoreHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FNsUIFocusTracker::HandleRestoreTick));
    }
}

int32 FNsUIFocusTracker::GetUserIndex() const
{
    const TSharedPtr<FSlateUser> User = Player.IsValid() ? Player->GetSlateUser() : nullptr;
    return User.IsValid() ? User->GetUserIndex() : INDEX_NONE;
}

void FNsUIFocusTracker::HandleFocusChanging(const FFocusEvent& Event, const FWeakWidgetPath& OldPath, const TSharedPtr<SWidget>& OldWidget, const FWidgetPath& NewPath, const TSharedPtr<SWidget>& NewWidget)
{
    if (Event.GetUser() != GetUserIndex())
    {
        return;
    }

    // Focus in another window, such as the editor, suspends the UI. Focus back in the game window resumes it
    if (NewWidget.IsValid())
    {
        SetSuspended(!IsInGameWindow(NewWidget));
    }

    // Slate sends this event before the focus moves. The report waits for the next frame, so a listener that reads the focus sees the new one
    if (!MovedHandle.IsValid())
    {
        MovedFrom = OldWidget;
        MovedHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FNsUIFocusTracker::HandleMovedTick));
    }
}

bool FNsUIFocusTracker::HandleMovedTick(float)
{
    MovedHandle.Reset();
    const TSharedPtr<FSlateUser> User = Player.IsValid() ? Player->GetSlateUser() : nullptr;
    const TSharedPtr<SWidget> Focus = User.IsValid() ? User->GetFocusedWidget() : nullptr;
    const TSharedPtr<SWidget> From = MovedFrom.Pin();
    MovedFrom.Reset();
    if (Focus != From)
    {
        OnFocusMoved.ExecuteIfBound(From, Focus);
    }
    return false;
}

void FNsUIFocusTracker::HandleActivationChanged(const bool bActive)
{
    if (!bActive)
    {
        SetSuspended(true);
        return;
    }
    const TSharedPtr<FSlateUser> User = Player.IsValid() ? Player->GetSlateUser() : nullptr;
    const TSharedPtr<SWidget> Focus = User.IsValid() ? User->GetFocusedWidget() : nullptr;
    SetSuspended(Focus.IsValid() && !IsInGameWindow(Focus));
}

bool FNsUIFocusTracker::HandleRestoreTick(float)
{
    RestoreHandle.Reset();
    if (!bSuspended)
    {
        OnRestore.ExecuteIfBound();
    }
    return false;
}

void FNsUIFocusTracker::SetSuspended(const bool bInSuspended)
{
    if (bSuspended == bInSuspended)
    {
        return;
    }
    bSuspended = bInSuspended;
    OnSuspendedChanged.ExecuteIfBound(bSuspended);
}

bool FNsUIFocusTracker::IsInGameWindow(const TSharedPtr<SWidget>& Widget) const
{
    const UGameViewportClient* const Viewport = Player.IsValid() ? Player->ViewportClient.Get() : nullptr;
    const TSharedPtr<SViewport> GameWidget = Viewport != nullptr ? Viewport->GetGameViewportWidget() : nullptr;
    if (!Widget.IsValid() || !GameWidget.IsValid() || !FSlateApplication::IsInitialized())
    {
        return false;
    }
    const TSharedPtr<SWindow> GameWindow = FSlateApplication::Get().FindWidgetWindow(GameWidget.ToSharedRef());
    const TSharedPtr<SWindow> WidgetWindow = FSlateApplication::Get().FindWidgetWindow(Widget.ToSharedRef());
    return GameWindow.IsValid() && GameWindow == WidgetWindow;
}
