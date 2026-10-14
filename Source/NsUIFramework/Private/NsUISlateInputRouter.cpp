#include "NsUISlateInputRouter.h"

#include "NsUIDeveloperSettings.h"
#include "NsUISubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/NavigationConfig.h"
#include "Blueprint/UserWidget.h"

namespace
{
TSharedPtr<FNsUISlateInputRouter> Router;
}

void FNsUISlateInputRouter::Startup()
{
    if (Router || !FSlateApplication::IsInitialized()) { return; }
    Router = MakeShared<FNsUISlateInputRouter>();
    auto& Slate = FSlateApplication::Get();
    Slate.RegisterInputPreProcessor(Router);
    Router->FocusHandle = Slate.OnFocusChanging().AddSP(Router.ToSharedRef(), &FNsUISlateInputRouter::OnFocusChanging);
    Router->ActivationHandle = Slate.OnApplicationActivationStateChanged().AddSP(Router.ToSharedRef(), &FNsUISlateInputRouter::OnActivationChanged);
}

void FNsUISlateInputRouter::Shutdown()
{
    if (Router && FSlateApplication::IsInitialized())
    {
        auto& Slate = FSlateApplication::Get();
        Slate.OnFocusChanging().Remove(Router->FocusHandle);
        Slate.OnApplicationActivationStateChanged().Remove(Router->ActivationHandle);
        Slate.UnregisterInputPreProcessor(Router);
    }
    Router.Reset();
}

void FNsUISlateInputRouter::ResetInputState(int32 UserIndex)
{
    if (Router) { Router->Users.Remove(UserIndex); }
}

UNsUISubsystem* FNsUISlateInputRouter::Resolve(int32 UserIndex)
{
    if (!GEngine) { return nullptr; }
    UNsUISubsystem* Fallback = nullptr;
    const auto Focus = FSlateApplication::Get().GetUserFocusedWidget(UserIndex);
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        UWorld* World = Context.World();
        if (!World || !World->IsGameWorld() || !World->GetGameInstance()) { continue; }
        for (ULocalPlayer* Player : World->GetGameInstance()->GetLocalPlayers())
        {
            UNsUISubsystem* Host = Player->GetSubsystem<UNsUISubsystem>();
            if (Host && Host->GetUserIndex() == UserIndex)
            {
                if (Host->IsGameFocus(Focus)) { return Host; }
                Fallback = Host;
            }
        }
    }
    return Fallback;
}

void FNsUISlateInputRouter::Tick(float, FSlateApplication&, TSharedRef<ICursor>) {}

bool FNsUISlateInputRouter::HandleKeyDownEvent(FSlateApplication& Slate, const FKeyEvent& Event)
{
    UNsUISubsystem* Host = Resolve(Event.GetUserIndex());
    if (!Host) { return false; }
    if (Event.GetKey() == EKeys::F1 && Event.IsShiftDown())
    {
        Host->SetSuspended(true);
        return false;
    }
    if (Host->bSuspended || !Host->Active) { return false; }
    auto& State = Users.FindOrAdd(Event.GetUserIndex());
    if (State.Consumed.Contains(Event.GetKey())) { return true; }
    const UNsUIDeveloperSettings* Settings = GetDefault<UNsUIDeveloperSettings>();
    const bool bConfirm = Settings->ConfirmKeys.Contains(Event.GetKey());
    const bool bCancel = Settings->CancelKeys.Contains(Event.GetKey());
    if (bConfirm || bCancel)
    {
        if (Event.IsRepeat()) { return true; }
        State.PressOwners.Add(Event.GetKey(), Host->PolicyOwner);
        if (Host->RouteAction(bConfirm))
        {
            Users.FindOrAdd(Event.GetUserIndex()).Consumed.Add(Event.GetKey());
            return true;
        }
        // Standard controls retain native Slate press and release handling.
        return false;
    }
    EUINavigation Direction = EUINavigation::Invalid;
    if (Settings->NavigateUpKeys.Contains(Event.GetKey())) { Direction = EUINavigation::Up; }
    else if (Settings->NavigateDownKeys.Contains(Event.GetKey())) { Direction = EUINavigation::Down; }
    else if (Settings->NavigateLeftKeys.Contains(Event.GetKey())) { Direction = EUINavigation::Left; }
    else if (Settings->NavigateRightKeys.Contains(Event.GetKey())) { Direction = EUINavigation::Right; }
    else if (Event.GetKey() == EKeys::Tab) { Direction = Event.IsShiftDown() ? EUINavigation::Previous : EUINavigation::Next; }
    else if (Settings->NavigateNextKeys.Contains(Event.GetKey())) { Direction = EUINavigation::Next; }
    else if (Settings->NavigatePreviousKeys.Contains(Event.GetKey())) { Direction = EUINavigation::Previous; }
    return Direction != EUINavigation::Invalid && Host->RouteNavigate(Direction, Event.GetKey().IsGamepadKey());
}

bool FNsUISlateInputRouter::HandleKeyUpEvent(FSlateApplication&, const FKeyEvent& Event)
{
    FUserState* State = Users.Find(Event.GetUserIndex());
    if (!State) { return false; }
    const bool bConsumed = State->Consumed.Remove(Event.GetKey()) > 0;
    TWeakObjectPtr<UUserWidget> Owner;
    const bool bHadOwner = State->PressOwners.RemoveAndCopyValue(Event.GetKey(), Owner);
    UNsUISubsystem* Host = Resolve(Event.GetUserIndex());
    return bConsumed || (bHadOwner && (!Host || Host->bSuspended || Owner != Host->PolicyOwner));
}

bool FNsUISlateInputRouter::HandleAnalogInputEvent(FSlateApplication& Slate, const FAnalogInputEvent& Event)
{
    UNsUISubsystem* Host = Resolve(Event.GetUserIndex());
    if (!Host || Host->bSuspended || !Host->Active) { return false; }
    if (Host->UsesNativeNavigation()) { return false; }
    const EUINavigation Direction = Slate.GetNavigationDirectionFromAnalog(Event);
    return Direction != EUINavigation::Invalid && Host->RouteNavigate(Direction, true);
}

void FNsUISlateInputRouter::OnFocusChanging(const FFocusEvent& Event, const FWeakWidgetPath&, const TSharedPtr<SWidget>&,
    const FWidgetPath&, const TSharedPtr<SWidget>& Widget)
{
    if (UNsUISubsystem* Host = Resolve(Event.GetUser()))
    {
        Host->ObserveFocus(Widget);
    }
}

void FNsUISlateInputRouter::OnActivationChanged(bool bActive)
{
    if (bActive)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (!Context.World() || !Context.World()->IsGameWorld() || !Context.World()->GetGameInstance()) { continue; }
            for (ULocalPlayer* Player : Context.World()->GetGameInstance()->GetLocalPlayers())
            {
                if (auto* Host = Player->GetSubsystem<UNsUISubsystem>())
                {
                    if (Host->IsGameFocus(FSlateApplication::Get().GetUserFocusedWidget(Host->GetUserIndex())))
                    {
                        Host->SetSuspended(false);
                    }
                }
            }
        }
    }
    if (!bActive)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.World() && Context.World()->GetGameInstance())
            {
                for (ULocalPlayer* Player : Context.World()->GetGameInstance()->GetLocalPlayers())
                {
                    if (auto* Host = Player->GetSubsystem<UNsUISubsystem>()) { Host->SetSuspended(true); }
                }
            }
        }
        Users.Reset();
    }
}

bool FNsUISlateInputRouter::HandleMouseMoveEvent(FSlateApplication& Slate, const FPointerEvent& Event)
{
    UNsUISubsystem* Host = Resolve(Event.GetUserIndex());
    if (!Host || Host->bSuspended || !Host->Active) { return false; }
    const FWidgetPath Path = Slate.LocateWindowUnderMouse(Event.GetScreenSpacePosition(), Slate.GetInteractiveTopLevelWindows());
    if (Path.IsValid() && Host->IsGameFocus(Path.GetLastWidget())) { Host->ObservePointer(Event.GetScreenSpacePosition()); }
    if (Path.IsValid() && Host->Contains(Path.GetLastWidget()))
    {
        Host->ObserveFocus(Path.GetLastWidget());
        if (!Host->bLogical) { Host->bRecover = true; }
    }
    return false;
}

bool FNsUISlateInputRouter::HandleMouseButtonDownEvent(FSlateApplication& Slate, const FPointerEvent& Event)
{
    UNsUISubsystem* Host = Resolve(Event.GetUserIndex());
    if (!Host) { return false; }
    const FWidgetPath Path = Slate.LocateWindowUnderMouse(Event.GetScreenSpacePosition(), Slate.GetInteractiveTopLevelWindows());
    if (Path.IsValid())
    {
        Host->SetSuspended(!Host->IsGameFocus(Path.GetLastWidget()));
        if (!Host->bSuspended && Host->Active && Event.GetEffectingButton() == EKeys::LeftMouseButton)
        {
            if (Host->ObservePointer(Event.GetScreenSpacePosition()))
            {
                Users.FindOrAdd(Event.GetUserIndex()).Consumed.Add(Event.GetEffectingButton());
                Host->RouteAction(true);
                return true;
            }
            if (Host->Contains(Path.GetLastWidget()))
            {
                Users.FindOrAdd(Event.GetUserIndex()).PressOwners.Add(Event.GetEffectingButton(), Host->PolicyOwner);
            }
        }
        if (!Host->bSuspended && Host->Active && !Host->Contains(Path.GetLastWidget()))
        {
            Host->bRecover = true;
        }
    }
    return false;
}

bool FNsUISlateInputRouter::HandleMouseButtonUpEvent(FSlateApplication&, const FPointerEvent& Event)
{
    FUserState* State = Users.Find(Event.GetUserIndex());
    if (!State) { return false; }
    const bool bConsumed = State->Consumed.Remove(Event.GetEffectingButton()) > 0;
    TWeakObjectPtr<UUserWidget> Owner;
    const bool bHadOwner = State->PressOwners.RemoveAndCopyValue(Event.GetEffectingButton(), Owner);
    UNsUISubsystem* Host = Resolve(Event.GetUserIndex());
    return bConsumed || (bHadOwner && (!Host || Host->bSuspended || Owner != Host->PolicyOwner));
}
