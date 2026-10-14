#pragma once

#include "Framework/Application/IInputProcessor.h"

class UNsUISubsystem;
class UUserWidget;

/** Routes Slate events to the local player host. */
class FNsUISlateInputRouter : public IInputProcessor
{
public:
    static void Startup();
    static void Shutdown();
    static void ResetInputState(int32 UserIndex);

    virtual void Tick(float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;
    virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& Event) override;
    virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& Event) override;
    virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& Event) override;
    virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& Event) override;
    virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& Event) override;
    virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& Event) override;

private:
    struct FUserState
    {
        TMap<FKey, TWeakObjectPtr<UUserWidget>> PressOwners;
        TSet<FKey> Consumed;
    };
    static UNsUISubsystem* Resolve(int32 UserIndex);
    void OnFocusChanging(const FFocusEvent& Event, const FWeakWidgetPath&, const TSharedPtr<SWidget>&, const FWidgetPath&, const TSharedPtr<SWidget>& Widget);
    void OnActivationChanged(bool bActive);
    TMap<int32, FUserState> Users;
    FDelegateHandle FocusHandle;
    FDelegateHandle ActivationHandle;
};
