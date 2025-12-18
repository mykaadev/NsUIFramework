#include "NsUISlateNavigation.h"

#include "NsUIDeveloperSettings.h"
#include "NsUISlateInputGuard.h"
#include "NsUISubsystem.h"
#include "Navigation/NsUIWidgetNavigationBehaviour.h"
#include "Widgets/SWidget.h"

namespace
{
    const UNsUIDeveloperSettings* GetSettings()
    {
        return GetDefault<UNsUIDeveloperSettings>();
    }

    bool KeyMatches(const FKeyEvent& InKeyEvent, const TArray<FKey>& Keys)
    {
        return Keys.Contains(InKeyEvent.GetKey());
    }

    EUINavigation DirectionFromKeys(const FKeyEvent& InKeyEvent)
    {
        const UNsUIDeveloperSettings* Settings = GetSettings();
        if (!Settings)
        {
            return EUINavigation::Invalid;
        }

        if (KeyMatches(InKeyEvent, Settings->NavigateUpKeys))
        {
            return EUINavigation::Up;
        }
        if (KeyMatches(InKeyEvent, Settings->NavigateDownKeys))
        {
            return EUINavigation::Down;
        }
        if (KeyMatches(InKeyEvent, Settings->NavigateLeftKeys))
        {
            return EUINavigation::Left;
        }
        if (KeyMatches(InKeyEvent, Settings->NavigateRightKeys))
        {
            return EUINavigation::Right;
        }
        if (KeyMatches(InKeyEvent, Settings->NavigateNextKeys))
        {
            return EUINavigation::Next;
        }
        if (KeyMatches(InKeyEvent, Settings->NavigatePreviousKeys))
        {
            return EUINavigation::Previous;
        }

        return EUINavigation::Invalid;
    }

    bool IsConfirmKey(const FKeyEvent& InKeyEvent)
    {
        const UNsUIDeveloperSettings* Settings = GetSettings();
        return Settings && KeyMatches(InKeyEvent, Settings->ConfirmKeys);
    }

    bool IsCancelKey(const FKeyEvent& InKeyEvent)
    {
        const UNsUIDeveloperSettings* Settings = GetSettings();
        return Settings && KeyMatches(InKeyEvent, Settings->CancelKeys);
    }

    // Slate treats user index 0 as the default keyboard/gamepad user; keep constant local for clarity.
    constexpr int32 KeyboardUserIndex = 0;

    /** Custom input pre-processor that forwards mouse clicks to NsUI confirm/cancel when active. */
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
            const UNsUIDeveloperSettings* Settings = GetSettings();
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
            if (const EUINavigation SettingsDirection = DirectionFromKeys(InKeyEvent); SettingsDirection != EUINavigation::Invalid)
            {
                if (UNsUISubsystem* Sys = Subsystem.Get())
                {
                    Sys->Navigate(SettingsDirection);
                }

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
                return EUINavigation::Invalid;
            }

            return EUINavigation::Invalid;
        }

        // NOTE: Navigation actions (Accept/Back) are forwarded into the subsystem as well so Confirm/Cancel
        // follow the same routing rules as directional navigation when NsUI is active.

        virtual EUINavigationAction GetNavigationActionFromKey(const FKeyEvent& InKeyEvent) const override
        {
            if (IsConfirmKey(InKeyEvent))
            {
                if (UNsUISubsystem* Sys = Subsystem.Get())
                {
                    Sys->Confirm();
                }
                return EUINavigationAction::Invalid;
            }

            if (IsCancelKey(InKeyEvent))
            {
                if (UNsUISubsystem* Sys = Subsystem.Get())
                {
                    Sys->Cancel();
                }
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
        TWeakObjectPtr<UNsUISubsystem> Subsystem;
        TSharedRef<FNavigationConfig> WrappedConfig;
    };
}

TSharedPtr<IInputProcessor> CreateNsUINavigationInputPreProcessor(UNsUISubsystem* Subsystem)
{
    return MakeShared<FNsUINavigationInputPreProcessor>(Subsystem);
}

TSharedRef<FNavigationConfig> CreateNsUINavigationConfig(UNsUISubsystem* Subsystem, const TSharedRef<FNavigationConfig>& WrappedConfig)
{
    return MakeShared<FNsUINavigationConfig>(Subsystem, WrappedConfig);
}
