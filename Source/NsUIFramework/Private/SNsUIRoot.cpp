#include "SNsUIRoot.h"

#include "Engine/Console.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SOverlay.h"

void SNsUIWidgetSlot::Construct(const FArguments& Args)
{
    OnNavigate = Args._OnNavigate;
    ChildSlot[Args._Content.Widget];
}

FNavigationReply SNsUIWidgetSlot::OnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& NavigationEvent)
{
    // The widget handler and the selection mode get each direction before the focus moves
    if (OnNavigate.IsBound() && OnNavigate.Execute(NavigationEvent.GetNavigationType()))
    {
        return FNavigationReply::Custom(FNavigationDelegate::CreateLambda([](EUINavigation) { return TSharedPtr<SWidget>(); }));
    }
    // The focus never leaves the open widget
    return FNavigationReply::Stop();
}

void SNsUIRoot::Construct(const FArguments& Args)
{
    Player = Args._Player;
    OnFindAction = Args._OnFindAction;
    OnAction = Args._OnAction;
    OnNavigate = Args._OnNavigate;
    OnPointer = Args._OnPointer;
    OnDevice = Args._OnDevice;

    TSharedRef<SOverlay> LayerStack = SNew(SOverlay);
    for (int32 Index = 0; Index <= static_cast<int32>(ENsUILayer::Modal); ++Index)
    {
        TSharedRef<SOverlay> Layer = SNew(SOverlay);
        Layers.Add(Layer);
        LayerStack->AddSlot()[Layer];
    }
    SetVisibility(EVisibility::SelfHitTestInvisible);
    ChildSlot[LayerStack];
}

FReply SNsUIRoot::OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
    OnDevice.ExecuteIfBound(ENsUIPointerMode::Keys, KeyEvent.GetKey().IsGamepadKey());
    if (IsConsoleOpen() || !OnFindAction.IsBound())
    {
        return FReply::Unhandled();
    }

    const TOptional<ENsUIAction> Action = OnFindAction.Execute(KeyEvent.GetKey());
    if (!Action.IsSet() || (Action.GetValue() != ENsUIAction::Confirm && Action.GetValue() != ENsUIAction::Back))
    {
        return FReply::Unhandled();
    }

    // A text box keeps the keyboard keys that it types
    const bool bGamepad = KeyEvent.GetKey().IsGamepadKey();
    if (!bGamepad && IsTextFocused(KeyEvent.GetUserIndex()))
    {
        return FReply::Unhandled();
    }

    // A held Confirm or Back does not repeat the action
    if (KeyEvent.IsRepeat())
    {
        return FReply::Handled();
    }

    if (Action.GetValue() == ENsUIAction::Confirm && OnAction.IsBound() && OnAction.Execute(ENsUIAction::Confirm, true, bGamepad))
    {
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

FReply SNsUIRoot::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
    if (IsConsoleOpen())
    {
        return FReply::Unhandled();
    }

    const TOptional<ENsUIAction> Action = OnFindAction.IsBound() ? OnFindAction.Execute(KeyEvent.GetKey()) : TOptional<ENsUIAction>();
    if (Action.IsSet() && !KeyEvent.IsRepeat())
    {
        const ENsUIAction Value = Action.GetValue();
        const bool bRouted = Value == ENsUIAction::Confirm || Value == ENsUIAction::Back || Value == ENsUIAction::NextTab || Value == ENsUIAction::PreviousTab;
        if (bRouted && OnAction.IsBound() && OnAction.Execute(Value, false, KeyEvent.GetKey().IsGamepadKey()))
        {
            return FReply::Handled();
        }
    }

    // The default turns a direction key into navigation while the root itself has the focus
    return SCompoundWidget::OnKeyDown(MyGeometry, KeyEvent);
}

FNavigationReply SNsUIRoot::OnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& NavigationEvent)
{
    if (OnNavigate.IsBound() && OnNavigate.Execute(NavigationEvent.GetNavigationType()))
    {
        return FNavigationReply::Custom(FNavigationDelegate::CreateLambda([](EUINavigation) { return TSharedPtr<SWidget>(); }));
    }
    return FNavigationReply::Stop();
}

FReply SNsUIRoot::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (!MouseEvent.GetCursorDelta().IsNearlyZero())
    {
        OnDevice.ExecuteIfBound(ENsUIPointerMode::Mouse, false);
        if (OnPointer.IsBound())
        {
            OnPointer.Execute(MouseEvent, false);
        }
    }
    return FReply::Unhandled();
}

FReply SNsUIRoot::OnPreviewMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    OnDevice.ExecuteIfBound(ENsUIPointerMode::Mouse, false);
    return FReply::Unhandled();
}

FReply SNsUIRoot::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (OnPointer.IsBound() && OnPointer.Execute(MouseEvent, true))
    {
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

TSharedRef<SNsUIWidgetSlot> SNsUIRoot::AddWidget(const ENsUILayer Layer, const TSharedRef<SWidget>& Content)
{
    TSharedRef<SNsUIWidgetSlot> Slot = SNew(SNsUIWidgetSlot).OnNavigate(OnNavigate)[Content];
    Layers[static_cast<int32>(Layer)]->AddSlot()[Slot];
    return Slot;
}

void SNsUIRoot::RemoveWidget(const TSharedRef<SWidget>& Slot)
{
    for (const TSharedRef<SOverlay>& Layer : Layers)
    {
        Layer->RemoveSlot(Slot);
    }
}

void SNsUIRoot::SetSlotInteractive(const TSharedRef<SWidget>& Slot, const bool bInteractive)
{
    Slot->SetVisibility(bInteractive ? EVisibility::SelfHitTestInvisible : EVisibility::HitTestInvisible);
}

void SNsUIRoot::SetCatchesPointer(const bool bCatches)
{
    SetVisibility(bCatches ? EVisibility::Visible : EVisibility::SelfHitTestInvisible);
}

bool SNsUIRoot::IsConsoleOpen() const
{
    const UGameViewportClient* const Viewport = Player.IsValid() ? Player->ViewportClient.Get() : nullptr;
    return Viewport != nullptr && Viewport->ViewportConsole != nullptr && Viewport->ViewportConsole->ConsoleState != NAME_None;
}

bool SNsUIRoot::IsTextFocused(const int32 UserIndex)
{
    if (!FSlateApplication::IsInitialized())
    {
        return false;
    }
    const TSharedPtr<SWidget> Focus = FSlateApplication::Get().GetUserFocusedWidget(UserIndex);
    const FName Type = Focus.IsValid() ? Focus->GetType() : NAME_None;
    return Type == TEXT("SEditableText") || Type == TEXT("SMultiLineEditableText");
}
