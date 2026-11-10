#include "Framework/Application/SlateApplication.h"
#include "Misc/AutomationTest.h"
#include "SNsUIRoot.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"

#if WITH_DEV_AUTOMATION_TESTS

/** A window with an NsUI root, one button, and one text box. It counts the actions that reach the root */
class FNsUIRootTestBench
{
// Functions
public:
    /** Constructor */
    FNsUIRootTestBench()
        : BackCount(0)
        , PreviewConfirmCount(0)
        , NavigateCount(0)
        , ClickCount(0)
        , bHandleConfirm(false)
        , bHandleNavigate(false)
    {
        Root = SNew(SNsUIRoot)
            .OnFindAction(FNsUIFindAction::CreateStatic(&FNsUIRootTestBench::FindAction))
            .OnAction(FNsUIRouteAction::CreateRaw(this, &FNsUIRootTestBench::RouteAction))
            .OnNavigate(FNsUIRouteNavigate::CreateRaw(this, &FNsUIRootTestBench::RouteNavigate));
        TSharedRef<SVerticalBox> Content = SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[SAssignNew(Button, SButton).OnClicked_Raw(this, &FNsUIRootTestBench::HandleClick)]
            + SVerticalBox::Slot().AutoHeight()[SAssignNew(Text, SEditableText)]
            + SVerticalBox::Slot().AutoHeight()[SAssignNew(Lower, SButton)];
        SNsUIRoot::SetSlotInteractive(Root->AddWidget(ENsUILayer::Menu, Content), true);
        Window = SNew(SWindow).ClientSize(FVector2D(400.f, 300.f)).SupportsMaximize(false).SupportsMinimize(false)[Root.ToSharedRef()];
        FSlateApplication::Get().AddWindow(Window.ToSharedRef(), true);
        FSlateApplication::Get().Tick();
    }

    /** Destroys the window */
    ~FNsUIRootTestBench()
    {
        FSlateApplication::Get().DestroyWindowImmediately(Window.ToSharedRef());
    }

    /** Gives the focus to a widget of the bench */
    void Focus(const TSharedPtr<SWidget>& Widget) const
    {
        FSlateApplication::Get().SetUserFocus(0, Widget, EFocusCause::SetDirectly);
    }

    /** Sends a key press and release to the focus */
    static void Press(const FKey& Key)
    {
        const FKeyEvent Event(Key, FModifierKeysState(), 0, false, 0, 0);
        FSlateApplication::Get().ProcessKeyDownEvent(Event);
        FSlateApplication::Get().ProcessKeyUpEvent(Event);
    }

private:
    /** The test keys: Escape and BackSpace are Back, E and Enter are Confirm, the arrows are directions */
    static TOptional<ENsUIAction> FindAction(const FKey& Key)
    {
        if (Key == EKeys::Escape || Key == EKeys::BackSpace || Key == EKeys::Gamepad_FaceButton_Right)
        {
            return ENsUIAction::Back;
        }
        if (Key == EKeys::E || Key == EKeys::Enter || Key == EKeys::Gamepad_FaceButton_Bottom)
        {
            return ENsUIAction::Confirm;
        }
        return TOptional<ENsUIAction>();
    }

    /** Counts the actions */
    bool RouteAction(const ENsUIAction Action, const bool bPreview, const bool bGamepad)
    {
        if (Action == ENsUIAction::Back)
        {
            ++BackCount;
            return true;
        }
        if (Action == ENsUIAction::Confirm && bPreview)
        {
            ++PreviewConfirmCount;
            return bHandleConfirm;
        }
        return false;
    }

    /** Counts the directions */
    bool RouteNavigate(const EUINavigation Direction)
    {
        ++NavigateCount;
        return bHandleNavigate;
    }

    /** Counts the clicks of the button */
    FReply HandleClick()
    {
        ++ClickCount;
        return FReply::Handled();
    }

// Variables
public:
    /** The root under test */
    TSharedPtr<SNsUIRoot> Root;

    /** The window of the root */
    TSharedPtr<SWindow> Window;

    /** The button */
    TSharedPtr<SButton> Button;

    /** The button under the text box */
    TSharedPtr<SButton> Lower;

    /** The text box */
    TSharedPtr<SEditableText> Text;

    /** The number of Back actions */
    int32 BackCount;

    /** The number of Confirm actions before the focused control */
    int32 PreviewConfirmCount;

    /** The number of directions */
    int32 NavigateCount;

    /** The number of button clicks */
    int32 ClickCount;

    /** True makes the handler use Confirm */
    bool bHandleConfirm;

    /** True makes the handler use the directions */
    bool bHandleNavigate;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNsUIRootTextTest, "NsUIFramework.Root.TextBoxKeepsKeys", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNsUIRootTextTest::RunTest(const FString& Parameters)
{
    FNsUIRootTestBench Bench;
    Bench.Focus(Bench.Text);
    if (!TestTrue(TEXT("The text box has the focus"), SNsUIRoot::IsTextFocused(0)))
    {
        return false;
    }
    Bench.Text->SetText(FText::FromString(TEXT("ab")));
    Bench.Text->SelectAllText();
    FSlateApplication::Get().ProcessKeyCharEvent(FCharacterEvent(TEXT('e'), FModifierKeysState(), 0, false));
    FNsUIRootTestBench::Press(EKeys::E);
    FNsUIRootTestBench::Press(EKeys::BackSpace);
    TestEqual(TEXT("Backspace stays in the text box"), Bench.BackCount, 0);
    TestEqual(TEXT("E stays in the text box"), Bench.PreviewConfirmCount, 0);
    TestEqual(TEXT("The text box got the keys"), Bench.Text->GetText().ToString(), FString());

    FNsUIRootTestBench::Press(EKeys::Gamepad_FaceButton_Bottom);
    TestEqual(TEXT("A gamepad Confirm still reaches the handler"), Bench.PreviewConfirmCount, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNsUIRootActionTest, "NsUIFramework.Root.ConfirmAndBack", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNsUIRootActionTest::RunTest(const FString& Parameters)
{
    FNsUIRootTestBench Bench;
    Bench.Focus(Bench.Button);

    FNsUIRootTestBench::Press(EKeys::Escape);
    TestEqual(TEXT("Back reaches the root from a button"), Bench.BackCount, 1);

    FNsUIRootTestBench::Press(EKeys::Enter);
    TestEqual(TEXT("The handler sees Confirm first"), Bench.PreviewConfirmCount, 1);

    Bench.bHandleConfirm = true;
    FNsUIRootTestBench::Press(EKeys::Enter);
    TestEqual(TEXT("A Confirm that the handler used never reaches the button"), Bench.ClickCount, 0);
    TestEqual(TEXT("The handler saw the second Confirm"), Bench.PreviewConfirmCount, 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNsUIRootNavigateTest, "NsUIFramework.Root.NavigateHandler", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNsUIRootNavigateTest::RunTest(const FString& Parameters)
{
    FNsUIRootTestBench Bench;
    Bench.Focus(Bench.Lower);
    Bench.bHandleNavigate = true;
    FNsUIRootTestBench::Press(EKeys::Up);
    TestEqual(TEXT("The handler sees the direction"), Bench.NavigateCount, 1);
    TestEqual(TEXT("A direction that the handler used does not move the focus"), FSlateApplication::Get().GetUserFocusedWidget(0), StaticCastSharedPtr<SWidget>(Bench.Lower));
    return true;
}

#endif
