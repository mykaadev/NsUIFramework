#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "UObject/Package.h"
#include "Misc/AutomationTest.h"
#include "NsUIWidgetStack.h"
#include "Tests/NsUITestWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

/** The owner of the widgets of the stack tests. A widget tree needs no world, so no world subsystem runs */
class FNsUIStackTestWidgets
{
// Functions
public:
    /** Constructor */
    FNsUIStackTestWidgets()
    {
        Tree = NewObject<UWidgetTree>(GetTransientPackage());
    }

    /** Makes a user widget */
    UUserWidget* MakeWidget() const
    {
        return CreateWidget<UUserWidget>(Tree, UNsUITestWidget::StaticClass());
    }

// Variables
private:
    /** The outer of the test widgets */
    UWidgetTree* Tree;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNsUIWidgetStackOrderTest, "NsUIFramework.Stack.Order", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNsUIWidgetStackOrderTest::RunTest(const FString& Parameters)
{
    FNsUIStackTestWidgets TestWorld;
    FNsUIWidgetStack Stack;
    UUserWidget* const Hud = TestWorld.MakeWidget();
    UUserWidget* const Pause = TestWorld.MakeWidget();
    UUserWidget* const Options = TestWorld.MakeWidget();
    UUserWidget* const Confirm = TestWorld.MakeWidget();
    if (!TestTrue(TEXT("The test widgets exist"), Hud != nullptr && Pause != nullptr && Options != nullptr && Confirm != nullptr))
    {
        return false;
    }

    FNsUIWidgetConfig Game;
    Game.Layer = ENsUILayer::Game;
    Game.InputPolicy = ENsUIInputPolicy::UIAndGame;
    FNsUIWidgetConfig Menu;
    Menu.Layer = ENsUILayer::Menu;
    FNsUIWidgetConfig Modal;
    Modal.Layer = ENsUILayer::Modal;

    UUserWidget* Previous = nullptr;
    Stack.Add(Hud, nullptr, Game);
    TestTrue(TEXT("The first widget becomes active"), Stack.UpdateActive(Previous));
    if (!TestNotNull(TEXT("A widget is active"), Stack.GetActive()))
    {
        return false;
    }
    TestEqual(TEXT("The Game widget is active"), Stack.GetActive()->Widget.Get(), Hud);

    Stack.Add(Confirm, nullptr, Modal);
    Stack.Add(Pause, nullptr, Menu);
    Stack.UpdateActive(Previous);
    TestEqual(TEXT("A Modal widget wins over a newer Menu widget"), Stack.GetActive()->Widget.Get(), Confirm);
    TestEqual(TEXT("The earlier active widget is reported"), Previous, Hud);

    Stack.Remove(Confirm);
    Stack.Add(Options, nullptr, Menu);
    Stack.UpdateActive(Previous);
    TestEqual(TEXT("In one layer the newest widget wins"), Stack.GetActive()->Widget.Get(), Options);
    TestNull(TEXT("A closed active widget is not reported as the earlier one"), Previous);

    Stack.Remove(Options);
    Stack.UpdateActive(Previous);
    TestEqual(TEXT("The next lower widget becomes active"), Stack.GetActive()->Widget.Get(), Pause);
    TestEqual(TEXT("The Menu layer lists one widget"), Stack.GetWidgetsOnLayer(ENsUILayer::Menu).Num(), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNsUIWidgetStackInputTest, "NsUIFramework.Stack.PassiveAndClosing", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNsUIWidgetStackInputTest::RunTest(const FString& Parameters)
{
    FNsUIStackTestWidgets TestWorld;
    FNsUIWidgetStack Stack;
    UUserWidget* const Menu = TestWorld.MakeWidget();
    UUserWidget* const Toast = TestWorld.MakeWidget();
    if (!TestTrue(TEXT("The test widgets exist"), Menu != nullptr && Toast != nullptr))
    {
        return false;
    }

    FNsUIWidgetConfig MenuConfig;
    FNsUIWidgetConfig ToastConfig;
    ToastConfig.Layer = ENsUILayer::Modal;
    ToastConfig.InputPolicy = ENsUIInputPolicy::Passive;

    UUserWidget* Previous = nullptr;
    Stack.Add(Menu, nullptr, MenuConfig);
    Stack.Add(Toast, nullptr, ToastConfig);
    Stack.UpdateActive(Previous);
    TestEqual(TEXT("A passive widget never gets input"), Stack.GetActive()->Widget.Get(), Menu);

    Stack.Find(Menu)->bClosing = true;
    Stack.UpdateActive(Previous);
    TestNull(TEXT("A widget in its outro gets no input"), Stack.GetActive());
    Stack.Find(Toast)->bClosing = true;
    TestNull(TEXT("A closing widget is not found by class"), Stack.FindNewestOfClass(UUserWidget::StaticClass()));
    return true;
}

#endif
