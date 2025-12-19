#include "NsUISlateNavigation.h"

#include "NsUIDeveloperSettings.h"
#include "NsUISlateInputGuard.h"
#include "NsUISubsystem.h"
#include "Navigation/NsUIWidgetNavigationBehaviour.h"
#include "Widgets/SWidget.h"

class FNsUINavigationHelpers
{
public:
    static const UNsUIDeveloperSettings* GetSettings()
    {
        return GetDefault<UNsUIDeveloperSettings>();
    }

    static EUINavigation DirectionFromKeys(const FKeyEvent& InKeyEvent)
    {
        const UNsUIDeveloperSettings* Settings = GetSettings();
        if (!Settings)
        {
            return EUINavigation::Invalid;
        }

        if (Settings->NavigateUpKeys.Contains(InKeyEvent.GetKey()))
        {
            return EUINavigation::Up;
        }
        if (Settings->NavigateDownKeys.Contains(InKeyEvent.GetKey()))
        {
            return EUINavigation::Down;
        }
        if (Settings->NavigateLeftKeys.Contains(InKeyEvent.GetKey()))
        {
            return EUINavigation::Left;
        }
        if (Settings->NavigateRightKeys.Contains(InKeyEvent.GetKey()))
        {
            return EUINavigation::Right;
        }
        if (Settings->NavigateNextKeys.Contains(InKeyEvent.GetKey()))
        {
            return EUINavigation::Next;
        }
        if (Settings->NavigatePreviousKeys.Contains(InKeyEvent.GetKey()))
        {
            return EUINavigation::Previous;
        }

        return EUINavigation::Invalid;
    }

    static bool IsConfirmKey(const FKeyEvent& InKeyEvent)
    {
        const UNsUIDeveloperSettings* Settings = GetSettings();
        return Settings && Settings->ConfirmKeys.Contains(InKeyEvent.GetKey());
    }

    static bool IsCancelKey(const FKeyEvent& InKeyEvent)
    {
        const UNsUIDeveloperSettings* Settings = GetSettings();
        return Settings && Settings->CancelKeys.Contains(InKeyEvent.GetKey());
    }
};

class FNsUINavigationInputPreProcessor : public IInputProcessor
{
public:
    explicit FNsUINavigationInputPreProcessor(TWeakObjectPtr<UNsUISubsystem> InSubsystem)
        : Subsystem(InSubsystem)
    {
    }

    virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
    {
        // No-op (we only care about mouse button events)
    }

    virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
    {
        UNsUISubsystem* Sys = Subsystem.Get();
        if (!Sys || !Sys->IsFrameworkActive() || IsForwardingSyntheticInputToSlate())
        {
            return false;
        }

        Sys->NoteMouseInput();

        const FKey Button = MouseEvent.GetEffectingButton();
        const UNsUIDeveloperSettings* Settings = FNsUINavigationHelpers::GetSettings();
        const bool bIsConfirmMouse = Settings && Settings->ConfirmMouseButtons.Contains(Button);
        if (bIsConfirmMouse)
        {
            if (UNsUINavigationBehaviour* Behaviour = Sys->GetActiveNavigationBehaviour())
            {
                if (Behaviour->OnMouseButtonDown(MouseEvent.GetScreenSpacePosition(), Button))
                {
                    return true;
                }
            }

            if (UNsUIWidgetNavigationBehaviour* WidgetBehaviour = Cast<UNsUIWidgetNavigationBehaviour>(Sys->GetActiveNavigationBehaviour()))
            {
                if (UWidget* TargetWidget = WidgetBehaviour->FindWidgetUnderPointer(MouseEvent.GetScreenSpacePosition()))
                {
                    Sys->HandleHoverFocus(TargetWidget);
                }
            }

            // Let Slate and any other input handlers process the click normally.
            return false;
        }

        return false;
    }

    virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
    {
        FNsUINavigationConfig::MarkKeyReleased(InKeyEvent.GetKey());
        return false;
    }

private:
    TWeakObjectPtr<UNsUISubsystem> Subsystem;
};

/** Custom navigation config that can route nav events into UNsUISubsystem when active. */
class FNsUINavigationConfig : public FNavigationConfig
{
public:

    FNsUINavigationConfig(TWeakObjectPtr<UNsUISubsystem> InSubsystem, const TSharedRef<FNavigationConfig>& InWrappedConfig)
        : Subsystem(InSubsystem)
        , WrappedConfig(InWrappedConfig)
    {
        // Start with the same settings as the wrapped config (key mappings, flags, etc.)
        *static_cast<FNavigationConfig*>(this) = *InWrappedConfig;
    }

    virtual EUINavigation GetNavigationDirectionFromKey(const FKeyEvent& InKeyEvent) const override
    {
        if (IsForwardingSyntheticInputToSlate())
        {
            return WrappedConfig->GetNavigationDirectionFromKey(InKeyEvent);
        }

        if (InKeyEvent.IsRepeat())
        {
            return EUINavigation::Invalid;
        }

        if (WasKeyHandled(InKeyEvent.GetKey()))
        {
            return EUINavigation::Invalid;
        }

        if (const EUINavigation SettingsDirection = FNsUINavigationHelpers::DirectionFromKeys(InKeyEvent); SettingsDirection != EUINavigation::Invalid)
        {
            if (UNsUISubsystem* Sys = Subsystem.Get())
            {
                Sys->Navigate(SettingsDirection);
            }

            MarkKeyHandled(InKeyEvent.GetKey());
            return EUINavigation::Invalid;
        }

        const EUINavigation Direction = WrappedConfig->GetNavigationDirectionFromKey(InKeyEvent);

        UNsUISubsystem* Sys = Subsystem.Get();
        if (!Sys || !Sys->IsFrameworkActive() || IsForwardingSyntheticInputToSlate())
        {
            // Framework inactive -> behave exactly like the original config.
            return Direction;
        }

        if (Direction != EUINavigation::Invalid)
        {
            Sys->Navigate(Direction);
            // Returning Invalid here tells Slate "we handled the nav; don't do built-in focus move".
            MarkKeyHandled(InKeyEvent.GetKey());
            return EUINavigation::Invalid;
        }

        return EUINavigation::Invalid;
    }

    // NOTE: Navigation actions (Accept/Back) are forwarded into the subsystem as well so Confirm/Cancel
    // follow the same routing rules as directional navigation when NsUI is active.

    virtual EUINavigationAction GetNavigationActionFromKey(const FKeyEvent& InKeyEvent) const override
    {
        if (IsForwardingSyntheticInputToSlate())
        {
            return WrappedConfig->GetNavigationActionFromKey(InKeyEvent);
        }

        if (InKeyEvent.IsRepeat())
        {
            return EUINavigationAction::Invalid;
        }

        if (WasKeyHandled(InKeyEvent.GetKey()))
        {
            return EUINavigationAction::Invalid;
        }

        if (FNsUINavigationHelpers::IsConfirmKey(InKeyEvent))
        {
            if (UNsUISubsystem* Sys = Subsystem.Get())
            {
                Sys->Confirm();
            }
            MarkKeyHandled(InKeyEvent.GetKey());
            return EUINavigationAction::Invalid;
        }

        if (FNsUINavigationHelpers::IsCancelKey(InKeyEvent))
        {
            if (UNsUISubsystem* Sys = Subsystem.Get())
            {
                Sys->Cancel();
            }
            MarkKeyHandled(InKeyEvent.GetKey());
            return EUINavigationAction::Invalid;
        }

        const EUINavigationAction Action = WrappedConfig->GetNavigationActionFromKey(InKeyEvent);

        UNsUISubsystem* Sys = Subsystem.Get();
        if (!Sys || !Sys->IsFrameworkActive() || IsForwardingSyntheticInputToSlate())
        {
            return Action;
        }

        switch (Action)
        {
        case EUINavigationAction::Accept:
            Sys->Confirm();
            return EUINavigationAction::Invalid;

        case EUINavigationAction::Back:
            Sys->Cancel();
            return EUINavigationAction::Invalid;

        default:
            break;
        }

        return Action;
    }

private:
    static bool WasKeyHandled(const FKey& Key);
    static void MarkKeyHandled(const FKey& Key);
    static void MarkKeyReleased(const FKey& Key);

private:
    TWeakObjectPtr<UNsUISubsystem> Subsystem;
    TSharedRef<FNavigationConfig> WrappedConfig;

    static TSet<FKey> HandledKeys;
    static TSet<FKey> KeysPendingRelease;
};

TSet<FKey> FNsUINavigationConfig::HandledKeys;
TSet<FKey> FNsUINavigationConfig::KeysPendingRelease;

bool FNsUINavigationConfig::WasKeyHandled(const FKey& Key)
{
    if (HandledKeys.Contains(Key))
    {
        if (KeysPendingRelease.Contains(Key))
        {
            HandledKeys.Remove(Key);
            KeysPendingRelease.Remove(Key);
        }

        return true;
    }

    return false;
}

void FNsUINavigationConfig::MarkKeyHandled(const FKey& Key)
{
    HandledKeys.Add(Key);
    KeysPendingRelease.Remove(Key);
}

void FNsUINavigationConfig::MarkKeyReleased(const FKey& Key)
{
    if (HandledKeys.Contains(Key))
    {
        // Defer the release until after the current Slate key-up processing completes so the
        // navigation config does not re-trigger the mapped action for the same key.
        KeysPendingRelease.Add(Key);
    }
    else
    {
        KeysPendingRelease.Remove(Key);
    }
}

TSharedPtr<IInputProcessor> CreateNsUINavigationInputPreProcessor(UNsUISubsystem* Subsystem)
{
    return MakeShared<FNsUINavigationInputPreProcessor>(Subsystem);
}

TSharedRef<FNavigationConfig> CreateNsUINavigationConfig(UNsUISubsystem* Subsystem, const TSharedRef<FNavigationConfig>& WrappedConfig)
{
    return MakeShared<FNsUINavigationConfig>(Subsystem, WrappedConfig);
}
