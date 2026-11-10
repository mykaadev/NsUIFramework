#include "Blueprint/UserWidget.h"
#include "Misc/AutomationTest.h"
#include "NsUIWidgetStack.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNsUIWidgetStackOrderTest, "NsUIFramework.Stack.Order", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNsUIWidgetStackOrderTest::RunTest(const FString& Parameters)
{
    FNsUIWidgetStack Stack;
    UUserWidget* const Hud = NewObject<UUserWidget>(GetTransientPackage());
    UUserWidget* const Pause = NewObject<UUserWidget>(GetTransientPackage());
    UUserWidget* const Options = NewObject<UUserWidget>(GetTransientPackage());
    UUserWidget* const Confirm = NewObject<UUserWidget>(GetTransientPackage());

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
    FNsUIWidgetStack Stack;
    UUserWidget* const Menu = NewObject<UUserWidget>(GetTransientPackage());
    UUserWidget* const Toast = NewObject<UUserWidget>(GetTransientPackage());

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
    TestNull(TEXT("A closing widget is not found by class"), Stack.FindNewestOfClass(UUserWidget::StaticClass()));
    return true;
}

#endif
