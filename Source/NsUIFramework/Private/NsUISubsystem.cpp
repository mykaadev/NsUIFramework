#include "NsUISubsystem.h"

#include "NsUICursor.h"
#include "NsUILayout.h"
#include "NsUILayer.h"
#include "NsUIScreen.h"
#include "NsUIDeveloperSettings.h"
#include "NsUISlateInputRouter.h"
#include "Selection/NsUISelectionMode.h"
#include "Blueprint/GameViewportSubsystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Framework/Application/NavigationConfig.h"
#include "Types/NavigationMetaData.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SViewport.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/SCompoundWidget.h"

namespace
{
class SNsUIInputRoot : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SNsUIInputRoot) {}
        SLATE_DEFAULT_SLOT(FArguments, Content)
    SLATE_END_ARGS()

    void Construct(const FArguments& Args) { ChildSlot[Args._Content.Widget]; }
    virtual bool SupportsKeyboardFocus() const override { return true; }
};
}

struct UNsUISubsystem::FScreenEntry
{
    TStrongObjectPtr<UUserWidget> Widget;
    TStrongObjectPtr<UObject> Context;
    TStrongObjectPtr<UNsUISelectionMode> Mode;
    FNsUIScreenConfig Config;
    ESlateVisibility Visibility = ESlateVisibility::Visible;
    bool bOwned = false;
    bool bActivated = false;
    bool bInterface = false;
    TSharedPtr<FNavigationMetaData> OriginalNavigation;
    TWeakPtr<SWidget> NavigationRoot;
    TWeakObjectPtr<UWidget> LayoutTarget;
    bool bHadLayout = false;
};

TStatId UNsUISubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UNsUISubsystem, STATGROUP_Tickables);
}

void UNsUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    if (UGameViewportSubsystem* Viewport = UGameViewportSubsystem::Get())
    {
        AddedHandle = Viewport->OnWidgetAdded.AddUObject(this, &ThisClass::OnWidgetAdded);
        RemovedHandle = Viewport->OnWidgetRemoved.AddUObject(this, &ThisClass::OnWidgetRemoved);
    }
    CleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::OnWorldCleanup);
}

void UNsUISubsystem::Deinitialize()
{
    bShuttingDown = true;
    if (UGameViewportSubsystem* Viewport = UGameViewportSubsystem::Get())
    {
        Viewport->OnWidgetAdded.Remove(AddedHandle);
        Viewport->OnWidgetRemoved.Remove(RemovedHandle);
    }
    FWorldDelegates::OnWorldCleanup.Remove(CleanupHandle);
    const auto Snapshot = Screens;
    for (const auto& Entry : Snapshot)
    {
        RemoveEntry(Entry, Entry->bOwned);
    }
    FNsUISlateInputRouter::ResetInputState(GetUserIndex());
    RemovePresentation();
    if (Cursor) { Cursor->RemoveFromParent(); }
    Layout = nullptr;
    Cursor = nullptr;
    Super::Deinitialize();
}

void UNsUISubsystem::OnWorldCleanup(UWorld* World, bool, bool)
{
    const auto Snapshot = Screens;
    for (const auto& Entry : Snapshot)
    {
        if (Entry->Widget->GetWorld() == World)
        {
            RemoveEntry(Entry, Entry->bOwned);
        }
    }
    if (Layout && Layout->GetWorld() == World)
    {
        RemovePresentation();
        Layout = nullptr;
    }
    if (Cursor && Cursor->GetWorld() == World)
    {
        Cursor->RemoveFromParent();
        Cursor = nullptr;
    }
    FNsUISlateInputRouter::ResetInputState(GetUserIndex());
    bInitialDiscovery = true;
    bSuspended = false;
    // The next world has a new player controller. Its input mode must be set once, also when no screen is active
    bPolicyDirty = true;
    PolicyOwner.Reset();
}

void UNsUISubsystem::EnsurePresentation()
{
    APlayerController* Player = GetLocalPlayer()->GetPlayerController(GetWorld());
    if (!Player || !GetWorld()->IsGameWorld())
    {
        return;
    }
    if (!Layout)
    {
        Layout = CreateWidget<UNsUILayout>(Player);
        const TWeakObjectPtr<ULocalPlayer> LocalPlayer = GetLocalPlayer();
        Presentation = SNew(SNsUIInputRoot)
            [SNew(SConstraintCanvas)
            + SConstraintCanvas::Slot()
            .Anchors_Lambda([LocalPlayer]()
            {
                return LocalPlayer.IsValid()
                    ? FAnchors(LocalPlayer->Origin.X, LocalPlayer->Origin.Y,
                        LocalPlayer->Origin.X + LocalPlayer->Size.X, LocalPlayer->Origin.Y + LocalPlayer->Size.Y)
                    : FAnchors(0.f, 0.f, 1.f, 1.f);
            })
            .Offset(FMargin(0.f))
            [Layout->TakeWidget()]];
        PresentationViewport = GetWorld()->GetGameViewport();
        PresentationZOrder = 0;
        PresentationViewport->AddViewportWidgetContent(Presentation.ToSharedRef(), PresentationZOrder + 10);
    }
    if (!Cursor)
    {
        Cursor = CreateWidget<UNsUICursor>(Player);
        Cursor->SetCursorBrush(Layout->GetCursorBrush());
        Cursor->AddToViewport(MAX_int32 - 100);
        Cursor->SetAlignmentInViewport(FVector2D(1.f, .5f));
        Cursor->SetDesiredSizeInViewport(FVector2D(16.f, 16.f));
    }
}

void UNsUISubsystem::RemovePresentation()
{
    if (Presentation && PresentationViewport.IsValid())
    {
        PresentationViewport->RemoveViewportWidgetContent(Presentation.ToSharedRef());
    }
    Presentation.Reset();
    PresentationViewport.Reset();
}

UUserWidget* UNsUISubsystem::OpenScreen(TSubclassOf<UUserWidget> Class, UObject* Context, ENsUILayer Layer)
{
    if (!Class || bShuttingDown)
    {
        return nullptr;
    }
    EnsurePresentation();
    if (!Layout)
    {
        return nullptr;
    }
    UUserWidget* Widget = CreateWidget<UUserWidget>(GetLocalPlayer()->GetPlayerController(GetWorld()), Class);
    if (Widget)
    {
        Layout->GetLayerWidget(Layer)->AddScreen(Widget);
        AddEntry(Widget, Context, Layer, true);
    }
    return Widget;
}

void UNsUISubsystem::AddEntry(UUserWidget* Widget, UObject* Context, ENsUILayer Layer, bool bOwned)
{
    if (bShuttingDown || Screens.ContainsByPredicate([&](const auto& Entry) { return Entry->Widget.Get() == Widget; }))
    {
        return;
    }
    const auto Entry = MakeShared<FScreenEntry>();
    Entry->Widget.Reset(Widget);
    Entry->Context.Reset(Context);
    Entry->bOwned = bOwned;
    Entry->bInterface = Widget->Implements<UNsUIScreen>();
    Entry->Config = Entry->bInterface ? INsUIScreen::Execute_GetNsScreenConfig(Widget) : FNsUIScreenConfig();
    if (bOwned) { Entry->Config.Layer = Layer; }
    Entry->Visibility = Widget->GetVisibility();
    UClass* ModeClass = Entry->Config.SelectionMode;
    if (!ModeClass || ModeClass->HasAnyClassFlags(CLASS_Abstract))
    {
        ModeClass = UNsUIUMGSelectionMode::StaticClass();
    }
    Entry->Mode.Reset(NewObject<UNsUISelectionMode>(Widget, ModeClass));
    Entry->Mode->Initialize(Widget);
    Screens.Add(Entry);
    // Set only the screen boundary. Child navigation rules remain unchanged.
    const TSharedPtr<SWidget> Root = Widget->GetCachedWidget();
    if (Root)
    {
        Entry->NavigationRoot = Root;
        Entry->OriginalNavigation = Root->GetMetaData<FNavigationMetaData>();
        auto Boundary = Entry->OriginalNavigation ? MakeShared<FNavigationMetaData>(*Entry->OriginalNavigation) : MakeShared<FNavigationMetaData>();
        for (uint8 Direction = 0; Direction < static_cast<uint8>(EUINavigation::Num); ++Direction)
        {
            if (Boundary->GetBoundaryRule(static_cast<EUINavigation>(Direction)) == EUINavigationRule::Escape)
            {
                Boundary->SetNavigationStop(static_cast<EUINavigation>(Direction));
            }
        }
        if (Entry->OriginalNavigation) { Root->RemoveMetaData(Entry->OriginalNavigation.ToSharedRef()); }
        Root->AddMetadata(Boundary);
    }
    if (Entry->bInterface)
    {
        INsUIScreen::Execute_OnNsScreenOpened(Widget, Context, Entry->Mode.Get());
    }
    Reconcile();
}

void UNsUISubsystem::OnWidgetAdded(UWidget* Widget, ULocalPlayer* Player)
{
    UUserWidget* Screen = Cast<UUserWidget>(Widget);
    if (!Screen || Screen->GetWorld() != GetWorld() || !GetWorld()->IsGameWorld() || !Screen->Implements<UNsUIScreen>())
    {
        return;
    }
    ULocalPlayer* Owner = Player ? Player : Screen->GetOwningLocalPlayer();
    if (Owner != GetLocalPlayer())
    {
        return;
    }
    AddEntry(Screen, nullptr, ENsUILayer::Menu, false);
}

void UNsUISubsystem::OnWidgetRemoved(UWidget* Widget)
{
    const auto* Found = Screens.FindByPredicate([&](const auto& Entry) { return Entry->Widget.Get() == Widget; });
    if (Found)
    {
        const auto Entry = *Found;
        RemoveEntry(Entry, false);
    }
}

void UNsUISubsystem::CloseScreen(UUserWidget* Widget)
{
    const auto* Found = Screens.FindByPredicate([&](const auto& Entry) { return Entry->Widget.Get() == Widget && Entry->bOwned; });
    if (Found)
    {
        const auto Entry = *Found;
        RemoveEntry(Entry, true);
    }
}

void UNsUISubsystem::CloseAll(ENsUILayer Layer)
{
    const auto Snapshot = Screens;
    for (const auto& Entry : Snapshot)
    {
        if (Entry->bOwned && Entry->Config.Layer == Layer)
        {
            RemoveEntry(Entry, true);
        }
    }
}

void UNsUISubsystem::RemoveEntry(const TSharedPtr<FScreenEntry>& Entry, bool bRemoveWidget)
{
    if (!Screens.Remove(Entry)) { return; }
    if (const auto Root = Entry->NavigationRoot.Pin())
    {
        if (const auto Current = Root->GetMetaData<FNavigationMetaData>()) { Root->RemoveMetaData(Current.ToSharedRef()); }
        if (Entry->OriginalNavigation) { Root->AddMetadata(Entry->OriginalNavigation.ToSharedRef()); }
    }
    Entry->Widget->SetVisibility(Entry->Visibility);
    if (Entry->bInterface)
    {
        INsUIScreen::Execute_OnNsScreenClosed(Entry->Widget.Get());
    }
    if (bRemoveWidget) { Entry->Widget->RemoveFromParent(); }
    Reconcile();
}

void UNsUISubsystem::Reconcile()
{
    if (bReconciling) { bReconcileAgain = true; return; }
    TGuardValue<bool> Guard(bReconciling, true);
    do
    {
        bReconcileAgain = false;
        TSharedPtr<FScreenEntry> Next;
        for (const auto& Entry : Screens)
        {
            if (Entry->Config.bInteractive && Entry->Widget->IsVisible() && Entry->Widget->GetIsEnabled()
                && (!Next || Entry->Config.Layer >= Next->Config.Layer))
            {
                Next = Entry;
            }
        }
        if (Active != Next)
        {
            const auto Previous = Active;
            if (FSlateApplication::IsInitialized() && GetUserIndex() != INDEX_NONE)
            {
                const auto Focus = FSlateApplication::Get().GetUserFocusedWidget(GetUserIndex());
                if (IsScreenPopup(Focus)) { FSlateApplication::Get().DismissMenuByWidget(Focus.ToSharedRef()); }
            }
            Active = Next;
            if (Previous && Screens.Contains(Previous) && Previous->bInterface)
            {
                INsUIScreen::Execute_OnNsScreenSuspended(Previous->Widget.Get());
            }
            if (Next && Screens.Contains(Next) && Next->bInterface && !bSuspended)
            {
                INsUIScreen::Execute_OnNsScreenActivated(Next->Widget.Get(), Next->bActivated);
                Next->bActivated = true;
            }
            bRecover = true;
            RecoveryReason = TEXT("Stack changed");
            ApplyInputPolicy();
        }
        for (const auto& Entry : Screens)
        {
            if (Entry->Config.bInteractive && Entry->Widget->IsVisible())
            {
                Entry->Widget->SetVisibility(Entry == Active ? Entry->Visibility : ESlateVisibility::HitTestInvisible);
            }
        }
        if (Presentation && PresentationViewport.IsValid())
        {
            UGameViewportSubsystem* Viewport = UGameViewportSubsystem::Get();
            int32 ZOrder = 0;
            if (Active && Active->bOwned)
            {
                for (const auto& Entry : Screens)
                {
                    if (!Entry->bOwned)
                    {
                        ZOrder = FMath::Max(ZOrder, FMath::Min(Viewport->GetWidgetSlot(Entry->Widget.Get()).ZOrder, MAX_int32 - 200) + 1);
                    }
                }
            }
            if (PresentationZOrder != ZOrder)
            {
                // Keep the Slate tree alive while changing its viewport order.
                PresentationViewport->RemoveViewportWidgetContent(Presentation.ToSharedRef());
                PresentationViewport->AddViewportWidgetContent(Presentation.ToSharedRef(), ZOrder + 10);
                PresentationZOrder = ZOrder;
            }
        }
    } while (bReconcileAgain);
}

void UNsUISubsystem::ApplyInputPolicy()
{
    UpdateNavigationConfig();
    UUserWidget* Owner = Active ? Active->Widget.Get() : nullptr;
    if (!bPolicyDirty && PolicyOwner.Get() == Owner) { return; }
    APlayerController* Player = GetLocalPlayer()->GetPlayerController(GetWorld());
    if (!Player) { return; }
    bPolicyDirty = false;
    PolicyOwner = Owner;
    if (!Active)
    {
        Player->SetInputMode(FInputModeGameOnly());
        Player->bShowMouseCursor = false;
    }
    else
    {
        const bool bGameplay = Active->Config.Gameplay == ENsUIGameplayPolicy::Allow
            || (Active->Config.Gameplay == ENsUIGameplayPolicy::Automatic && Active->Config.Layer == ENsUILayer::Game);
        if (bGameplay)
        {
            FInputModeGameAndUI Mode;
            Mode.SetHideCursorDuringCapture(false);
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            Player->SetInputMode(Mode);
        }
        else
        {
            FInputModeUIOnly Mode;
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            Player->SetInputMode(Mode);
        }
        Player->bShowMouseCursor = true;
    }
}

void UNsUISubsystem::UpdateNavigationConfig()
{
    const auto User = GetLocalPlayer()->GetSlateUser();
    if (!User) { return; }
    if (!Active || bSuspended || bShuttingDown)
    {
        if (Navigation && User->GetUserNavigationConfig() == Navigation)
        {
            User->SetUserNavigationConfig(PreviousNavigation);
        }
        Navigation.Reset();
        PreviousNavigation.Reset();
        return;
    }
    const bool bGameplay = Active->Config.Gameplay == ENsUIGameplayPolicy::Allow
        || (Active->Config.Gameplay == ENsUIGameplayPolicy::Automatic && Active->Config.Layer == ENsUILayer::Game);
    if (Navigation)
    {
        Navigation->bAnalogNavigation = !bGameplay;
        return;
    }
    PreviousNavigation = User->GetUserNavigationConfig();
    const auto Base = PreviousNavigation ? PreviousNavigation.ToSharedRef() : FSlateApplication::Get().GetNavigationConfig();
    Navigation = MakeShared<FNavigationConfig>(*Base);
    Navigation->bKeyNavigation = true;
    Navigation->bTabNavigation = true;
    Navigation->bAnalogNavigation = !bGameplay;
    Navigation->OnUserRemoved(GetUserIndex());
    const auto* Settings = GetDefault<UNsUIDeveloperSettings>();
    for (FKey Key : Settings->ConfirmKeys) { Navigation->KeyActionRules.Add(Key, EUINavigationAction::Accept); }
    for (FKey Key : Settings->CancelKeys) { Navigation->KeyActionRules.Add(Key, EUINavigationAction::Back); }
    const auto Add = [&](const TArray<FKey>& Keys, EUINavigation Direction)
    {
        for (FKey Key : Keys) { if (Key != EKeys::Tab) { Navigation->KeyEventRules.Add(Key, Direction); } }
    };
    Add(Settings->NavigateUpKeys, EUINavigation::Up);
    Add(Settings->NavigateDownKeys, EUINavigation::Down);
    Add(Settings->NavigateLeftKeys, EUINavigation::Left);
    Add(Settings->NavigateRightKeys, EUINavigation::Right);
    Add(Settings->NavigateNextKeys, EUINavigation::Next);
    Add(Settings->NavigatePreviousKeys, EUINavigation::Previous);
    User->SetUserNavigationConfig(Navigation);
}

int32 UNsUISubsystem::GetUserIndex() const
{
    const auto User = GetLocalPlayer() ? GetLocalPlayer()->GetSlateUser() : nullptr;
    return User ? User->GetUserIndex() : INDEX_NONE;
}

bool UNsUISubsystem::Contains(const TSharedPtr<SWidget>& Widget) const
{
    const auto Root = Active ? Active->Widget->GetCachedWidget() : nullptr;
    for (auto Current = Widget; Current; Current = Current->GetParentWidget())
    {
        if (Current == Root) { return true; }
    }
    return IsScreenPopup(Widget);
}

bool UNsUISubsystem::IsScreenPopup(const TSharedPtr<SWidget>& Widget) const
{
    if (!Active || !Widget) { return false; }
    auto& Slate = FSlateApplication::Get();
    const auto Root = Active->Widget->GetCachedWidget();
    for (auto Owner = Slate.GetMenuHostWidget(); Owner; Owner = Owner->GetParentWidget())
    {
        if (Owner == Root)
        {
            FWidgetPath Path;
            Slate.GeneratePathToWidgetUnchecked(Widget.ToSharedRef(), Path);
            return Path.IsValid() && Slate.FindMenuInWidgetPath(Path).IsValid();
        }
    }
    return false;
}

bool UNsUISubsystem::IsGameFocus(const TSharedPtr<SWidget>& Widget) const
{
    if (IsScreenPopup(Widget)) { return true; }
    const UGameViewportClient* Viewport = GetLocalPlayer()->ViewportClient;
    const TSharedPtr<SViewport> GameWidget = Viewport ? Viewport->GetGameViewportWidget() : nullptr;
    for (auto Current = Widget; Current; Current = Current->GetParentWidget())
    {
        if (Current == GameWidget) { return true; }
        for (const auto& Entry : Screens)
        {
            if (Current == Entry->Widget->GetCachedWidget()) { return true; }
        }
    }
    return false;
}

void UNsUISubsystem::SetSuspended(bool bSuspend)
{
    if (bSuspended == bSuspend) { return; }
    bSuspended = bSuspend;
    UpdateNavigationConfig();
    if (Active && Active->bInterface)
    {
        if (bSuspend) { INsUIScreen::Execute_OnNsScreenSuspended(Active->Widget.Get()); }
        else { INsUIScreen::Execute_OnNsScreenActivated(Active->Widget.Get(), true); }
    }
    if (!bSuspend)
    {
        bRecover = true;
        RecoveryReason = TEXT("Viewport activated");
        ApplyInputPolicy();
    }
    else if (Cursor) { Cursor->Present(TOptional<FVector2D>(), 0.f); }
}

void UNsUISubsystem::ObserveFocus(const TSharedPtr<SWidget>& Widget)
{
    if (!Active) { return; }
    if (Contains(Widget))
    {
        SetSuspended(false);
        if (!bLogical) { Active->Mode->ObserveFocus(Widget); }
    }
    else if (Widget && !IsGameFocus(Widget))
    {
        SetSuspended(true);
    }
    else if (Widget && IsGameFocus(Widget))
    {
        SetSuspended(false);
        bRecover = true;
        RecoveryReason = TEXT("Viewport focus restored");
    }
    else if (!bSuspended)
    {
        bRecover = true;
        RecoveryReason = TEXT("Focus left the active screen");
    }
}

void UNsUISubsystem::RecoverSelection()
{
    bRecover = false;
    const auto Entry = Active;
    if (!Entry || bSuspended || GetUserIndex() == INDEX_NONE) { return; }
    const FNsUISelectionRequest Request = Entry->bInterface
        ? INsUIScreen::Execute_GetNsScreenSelection(Entry->Widget.Get()) : FNsUISelectionRequest();
    bLogical = Request.Policy == ENsUISelectionPolicy::Logical;
    if (!bLogical && IsScreenPopup(FSlateApplication::Get().GetUserFocusedWidget(GetUserIndex()))) { return; }
    if (bLogical)
    {
        if (Presentation && FSlateApplication::Get().GetUserFocusedWidget(GetUserIndex()) != Presentation)
        {
            FSlateApplication::Get().SetUserFocus(GetUserIndex(), Presentation, EFocusCause::SetDirectly);
        }
        return;
    }
    if (Entry->Mode->Restore(Request.PreferredTarget))
    {
        if (UWidget* Target = Entry->Mode->GetFocusTarget())
        {
            if (FSlateApplication::Get().GetUserFocusedWidget(GetUserIndex()) != Target->GetCachedWidget())
            {
                FSlateApplication::Get().SetUserFocus(GetUserIndex(), Target->GetCachedWidget(), EFocusCause::Navigation);
            }
        }
        else if (Presentation && FSlateApplication::Get().GetUserFocusedWidget(GetUserIndex()) != Presentation)
        {
            FSlateApplication::Get().SetUserFocus(GetUserIndex(), Presentation, EFocusCause::SetDirectly);
        }
    }
}

void UNsUISubsystem::Tick(float DeltaTime)
{
    if (bShuttingDown || !GetWorld() || !GetWorld()->IsGameWorld() || !FSlateApplication::IsInitialized()) { return; }
    // A new world with no screen still needs its input mode once
    if (bPolicyDirty) { ApplyInputPolicy(); }
    if (bInitialDiscovery)
    {
        bInitialDiscovery = false;
        TArray<UUserWidget*> Existing;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), Existing, UUserWidget::StaticClass(), true);
        Existing.StableSort([](const UUserWidget& A, const UUserWidget& B)
        {
            return UGameViewportSubsystem::Get()->GetWidgetSlot(&A).ZOrder < UGameViewportSubsystem::Get()->GetWidgetSlot(&B).ZOrder;
        });
        for (UUserWidget* Widget : Existing) { OnWidgetAdded(Widget, Widget->GetOwningLocalPlayer()); }
    }
    const auto Snapshot = Screens;
    for (const auto& Entry : Snapshot)
    {
        if ((Entry->bOwned && !Entry->Widget->GetParent()) || (!Entry->bOwned && !Entry->Widget->IsInViewport()))
        {
            RemoveEntry(Entry, false);
        }
    }
    Reconcile();
    if (Active && !Navigation && !bSuspended) { ApplyInputPolicy(); }
    if (Active && !bSuspended)
    {
        EnsurePresentation();
        const auto Entry = Active;
        const FNsUISelectionRequest Request = Entry->bInterface
            ? INsUIScreen::Execute_GetNsScreenSelection(Entry->Widget.Get()) : FNsUISelectionRequest();
        if (bLogical != (Request.Policy == ENsUISelectionPolicy::Logical))
        {
            bRecover = true;
            RecoveryReason = TEXT("Selection policy changed");
        }
        RefreshElapsed += DeltaTime;
        if (RefreshElapsed >= .1f)
        {
            RefreshElapsed = 0.f;
            Entry->Mode->Refresh();
            if (!bLogical && !Entry->Mode->HasSelection())
            {
                bRecover = true;
                RecoveryReason = TEXT("Selection became unavailable");
            }
        }
        if (bRecover) { RecoverSelection(); }
        UWidget* Target = Entry->Mode->GetFocusTarget();
        if (Entry->LayoutTarget.Get() != Target)
        {
            Entry->LayoutTarget = Target;
            Entry->bHadLayout = false;
        }
        const bool bHasLayout = Target && !Target->GetCachedGeometry().GetLocalSize().IsNearlyZero();
        if (bHasLayout && !Entry->bHadLayout)
        {
            bRecover = true;
            RecoveryReason = TEXT("First target layout");
        }
        Entry->bHadLayout = bHasLayout;
    }
    if (Cursor)
    {
        FVector2D Position;
        const bool bVisible = Active && !bSuspended && !bLogical && Active->Mode->GetCursorPosition(Position);
        Cursor->Present(bVisible ? TOptional<FVector2D>(Position) : TOptional<FVector2D>(), DeltaTime);
    }
}

bool UNsUISubsystem::RouteAction(bool bConfirm)
{
    const auto Entry = Active;
    if (!Entry || bSuspended) { return false; }
    if (IsScreenPopup(FSlateApplication::Get().GetUserFocusedWidget(GetUserIndex()))) { return false; }
    if (Entry->bInterface && (bConfirm ? INsUIScreen::Execute_HandleNsScreenConfirm(Entry->Widget.Get())
        : INsUIScreen::Execute_HandleNsScreenCancel(Entry->Widget.Get()))) { return true; }
    if (Active != Entry || !Screens.Contains(Entry)) { return true; }
    if (bConfirm ? Entry->Mode->Confirm() : Entry->Mode->Cancel()) { return true; }
    if (!bConfirm && Entry->bOwned)
    {
        CloseScreen(Entry->Widget.Get());
        return true;
    }
    if (bLogical || !Entry->Mode->GetFocusTarget()) { return true; }
    if (!Contains(FSlateApplication::Get().GetUserFocusedWidget(GetUserIndex())))
    {
        bRecover = true;
        return true;
    }
    return false;
}

bool UNsUISubsystem::UsesNativeNavigation() const
{
    return Active && !bLogical && Active->Mode->UsesNativeNavigation();
}

bool UNsUISubsystem::ObservePointer(const FVector2D& AbsolutePosition)
{
    if (!Active || bSuspended || bLogical) { return false; }
    FVector2D Pixels, Position;
    USlateBlueprintLibrary::AbsoluteToViewport(GetWorld(), AbsolutePosition, Pixels, Position);
    return Active->Mode->ObservePointer(Position);
}

bool UNsUISubsystem::RouteNavigate(EUINavigation Direction, bool bGamepad)
{
    const auto Entry = Active;
    if (!Entry || bSuspended) { return false; }
    if (bLogical || Entry->Mode->Navigate(Direction)) { return true; }
    return !Entry->Mode->UsesNativeNavigation();
}

FString UNsUISubsystem::DescribeState() const
{
    FString Result = FString::Printf(TEXT("User: %d\nSuspended: %s\nRecovery: %s\n"), GetUserIndex(), bSuspended ? TEXT("Yes") : TEXT("No"), *RecoveryReason);
    for (int32 Index = Screens.Num() - 1; Index >= 0; --Index)
    {
        const auto& Entry = Screens[Index];
        Result += FString::Printf(TEXT("\n%s | Layer %d | %s | %s\n%s | %s\n"),
            *Entry->Widget->GetName(), static_cast<int32>(Entry->Config.Layer),
            Entry == Active ? (bLogical ? TEXT("Logical") : TEXT("Active")) : Entry->Config.bInteractive ? TEXT("Suspended") : TEXT("Passive"),
            Entry->bOwned ? TEXT("Host") : TEXT("Viewport"), *Entry->Mode->GetClass()->GetName(), *Entry->Mode->DescribeSelection());
    }
    return Result;
}
