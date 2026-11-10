#include "NsUISubsystem.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "NsUICursorWidget.h"
#include "NsUIFocusTracker.h"
#include "NsUIInputActions.h"
#include "NsUISettings.h"
#include "NsUIWidgetInterface.h"
#include "NsUIWidgetStack.h"
#include "SNsUIRoot.h"
#include "Selection/NsUISelectionMode.h"
#include "Slate/SObjectWidget.h"

UNsUISubsystem::UNsUISubsystem()
{
    PointerMode = ENsUIPointerMode::Mouse;
    bLastDeviceGamepad = false;
}

void UNsUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Stack = MakePimpl<FNsUIWidgetStack>();
    InputActions = MakePimpl<FNsUIInputActions>(GetLocalPlayer());
    FocusTracker = MakePimpl<FNsUIFocusTracker>(GetLocalPlayer(),
        FNsUIFocusTracker::FOnFocusMoved::CreateUObject(this, &ThisClass::HandleFocusMoved),
        FNsUIFocusTracker::FOnSuspendedChanged::CreateUObject(this, &ThisClass::HandleSuspendedChanged),
        FNsUIFocusTracker::FOnRestore::CreateUObject(this, &ThisClass::ApplyFocus));
    WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::HandleWorldCleanup);

    if (UEnhancedInputLocalPlayerSubsystem* const Input = Collection.InitializeDependency<UEnhancedInputLocalPlayerSubsystem>())
    {
        Input->ControlMappingsRebuiltDelegate.AddDynamic(this, &ThisClass::HandleMappingsRebuilt);
    }
}

void UNsUISubsystem::Deinitialize()
{
    FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
    for (UUserWidget* const Widget : Stack->Clear())
    {
        if (Widget != nullptr && Widget->Implements<UNsUIWidgetInterface>())
        {
            INsUIWidgetInterface::Execute_OnClosed(Widget);
        }
    }
    UpdateGameFirstInput(false);
    RemoveRoot();
    if (Cursor != nullptr)
    {
        Cursor->RemoveFromParent();
        Cursor = nullptr;
    }
    FocusTracker.Reset();
    InputActions.Reset();
    Super::Deinitialize();
}

void UNsUISubsystem::PlayerControllerChanged(APlayerController* const NewPlayerController)
{
    Super::PlayerControllerChanged(NewPlayerController);
    if (NewPlayerController == nullptr)
    {
        return;
    }
    InputActions->AddMappingContext();
    ApplyInputPolicy();
}

void UNsUISubsystem::AddReferencedObjects(UObject* const InThis, FReferenceCollector& Collector)
{
    Super::AddReferencedObjects(InThis, Collector);
    UNsUISubsystem* const This = CastChecked<UNsUISubsystem>(InThis);
    if (This->Stack.IsValid())
    {
        This->Stack->AddReferencedObjects(Collector);
    }
}

UUserWidget* UNsUISubsystem::OpenWidget(const TSubclassOf<UUserWidget> WidgetClass, UObject* const Context, const ENsUILayer Layer)
{
    if (WidgetClass == nullptr)
    {
        UE_LOG(LogNsUI, Error, TEXT("OpenWidget got no widget class"));
        return nullptr;
    }
    APlayerController* const Player = GetPlayerController();
    if (Player == nullptr)
    {
        UE_LOG(LogNsUI, Warning, TEXT("OpenWidget %s: the player has no player controller in a game world yet"), *WidgetClass->GetName());
        return nullptr;
    }

    EnsureRoot();
    EnsureCursor();
    UUserWidget* const Widget = CreateWidget<UUserWidget>(Player, WidgetClass);
    if (Widget == nullptr)
    {
        UE_LOG(LogNsUI, Error, TEXT("OpenWidget could not make a widget of %s"), *WidgetClass->GetName());
        return nullptr;
    }

    const bool bInterface = Widget->Implements<UNsUIWidgetInterface>();
    FNsUIWidgetConfig Config = bInterface ? INsUIWidgetInterface::Execute_GetConfig(Widget) : FNsUIWidgetConfig();
    Config.Layer = Layer;

    UNsUISelectionMode* Mode = nullptr;
    if (Config.SelectionModeClass != nullptr && !Config.SelectionModeClass->HasAnyClassFlags(CLASS_Abstract))
    {
        Mode = NewObject<UNsUISelectionMode>(this, Config.SelectionModeClass);
        Mode->Initialize(Widget);
    }

    FNsUIWidgetEntry& Entry = Stack->Add(Widget, Context, Config);
    Entry.Mode = Mode;
    Entry.Slot = Root->AddWidget(Layer, Widget->TakeWidget());
    SNsUIRoot::SetSlotInteractive(Entry.Slot.ToSharedRef(), false);
    UE_LOG(LogNsUI, Verbose, TEXT("Open %s on layer %d"), *Widget->GetName(), static_cast<int32>(Layer));

    if (bInterface)
    {
        INsUIWidgetInterface::Execute_OnOpened(Widget, Context, Mode);
    }
    UpdateActive(false);
    return Widget;
}

void UNsUISubsystem::AddExternalWidget(UUserWidget* const Widget, const ENsUILayer Layer)
{
    if (Widget == nullptr || Stack->Find(Widget) != nullptr)
    {
        return;
    }
    FNsUIWidgetConfig Config = Widget->Implements<UNsUIWidgetInterface>() ? INsUIWidgetInterface::Execute_GetConfig(Widget) : FNsUIWidgetConfig();
    Config.Layer = Layer;
    if (Config.InputPolicy != ENsUIInputPolicy::GameFirst && Config.InputPolicy != ENsUIInputPolicy::Passive)
    {
        UE_LOG(LogNsUI, Error, TEXT("AddExternalWidget %s: an external widget must be GameFirst or Passive. Open it with OpenWidget to give it the focus"), *Widget->GetName());
        return;
    }
    Config.SelectionModeClass = nullptr;
    Stack->Add(Widget, nullptr, Config);
    if (Widget->Implements<UNsUIWidgetInterface>())
    {
        INsUIWidgetInterface::Execute_OnOpened(Widget, nullptr, nullptr);
    }
    UpdateActive(false);
}

bool UNsUISubsystem::RemoveExternalWidget(UUserWidget* const Widget)
{
    if (!IsExternalWidget(Widget))
    {
        return false;
    }
    RemoveEntry(Widget);
    return true;
}

bool UNsUISubsystem::IsExternalWidget(const UUserWidget* const Widget) const
{
    const FNsUIWidgetEntry* const Entry = Stack.IsValid() ? Stack->Find(Widget) : nullptr;
    return Entry != nullptr && !Entry->Slot.IsValid();
}

void UNsUISubsystem::CloseWidget(UUserWidget* const Widget)
{
    FNsUIWidgetEntry* const Entry = Stack->Find(Widget);
    if (Entry == nullptr || Entry->bClosing)
    {
        return;
    }
    if (!Entry->Slot.IsValid())
    {
        UE_LOG(LogNsUI, Warning, TEXT("CloseWidget %s: the widget is external. The system that shows it removes it"), *Widget->GetName());
        return;
    }
    if (Widget->Implements<UNsUIWidgetInterface>() && INsUIWidgetInterface::Execute_BeginClose(Widget))
    {
        Entry->bClosing = true;
        UpdateActive(false);
        return;
    }
    RemoveEntry(Widget);
}

void UNsUISubsystem::FinishCloseWidget(UUserWidget* const Widget)
{
    if (Stack->Find(Widget) != nullptr)
    {
        RemoveEntry(Widget);
    }
}

void UNsUISubsystem::CloseAll(const ENsUILayer Layer)
{
    for (UUserWidget* const Widget : Stack->GetWidgetsOnLayer(Layer))
    {
        CloseWidget(Widget);
    }
}

UUserWidget* UNsUISubsystem::ToggleWidget(const TSubclassOf<UUserWidget> WidgetClass, UObject* const Context, const ENsUILayer Layer)
{
    if (UUserWidget* const Open = FindWidget(WidgetClass))
    {
        CloseWidget(Open);
        return nullptr;
    }
    return OpenWidget(WidgetClass, Context, Layer);
}

UUserWidget* UNsUISubsystem::FindWidget(const TSubclassOf<UUserWidget> WidgetClass) const
{
    const FNsUIWidgetEntry* const Entry = Stack.IsValid() ? Stack->FindNewestOfClass(WidgetClass) : nullptr;
    return Entry != nullptr ? Entry->Widget.Get() : nullptr;
}

bool UNsUISubsystem::IsWidgetOpen(const UUserWidget* const Widget) const
{
    const FNsUIWidgetEntry* const Entry = Stack.IsValid() ? Stack->Find(Widget) : nullptr;
    return Entry != nullptr && !Entry->bClosing;
}

UUserWidget* UNsUISubsystem::GetActiveWidget() const
{
    const FNsUIWidgetEntry* const Entry = Stack.IsValid() ? Stack->GetActive() : nullptr;
    return Entry != nullptr ? Entry->Widget.Get() : nullptr;
}

UNsUISelectionMode* UNsUISubsystem::GetSelectionMode(const UUserWidget* const Widget) const
{
    const FNsUIWidgetEntry* const Entry = Stack.IsValid() ? Stack->Find(Widget) : nullptr;
    return Entry != nullptr ? Entry->Mode.Get() : nullptr;
}

void UNsUISubsystem::RefreshConfig(UUserWidget* const Widget)
{
    FNsUIWidgetEntry* const Entry = Stack->Find(Widget);
    if (Entry == nullptr || !Widget->Implements<UNsUIWidgetInterface>())
    {
        return;
    }
    const ENsUILayer Layer = Entry->Config.Layer;
    const TSubclassOf<UNsUISelectionMode> ModeClass = Entry->Config.SelectionModeClass;
    Entry->Config = INsUIWidgetInterface::Execute_GetConfig(Widget);
    Entry->Config.Layer = Layer;
    if (Entry->Config.SelectionModeClass != ModeClass)
    {
        UE_LOG(LogNsUI, Warning, TEXT("RefreshConfig of %s cannot change the selection mode. It keeps the mode from the open"), *Widget->GetName());
        Entry->Config.SelectionModeClass = ModeClass;
    }
    UpdateActive(true);
}

void UNsUISubsystem::FocusChild(UUserWidget* const Widget, UWidget* const Child)
{
    FNsUIWidgetEntry* const Entry = Stack->Find(Widget);
    const TSharedPtr<SWidget> Target = Child != nullptr ? Child->GetCachedWidget() : nullptr;
    if (Entry == nullptr || !Target.IsValid() || !IsInside(Entry->Slot, Target))
    {
        UE_LOG(LogNsUI, Warning, TEXT("FocusChild: %s is not a child of an open widget"), *GetNameSafe(Child));
        return;
    }
    const TSharedPtr<SWidget> Focusable = CanFocus(Target) ? Target : FindFirstFocusable(Target.ToSharedRef());
    Entry->LastFocus = Focusable;
    if (Focusable.IsValid() && Entry == Stack->GetActive() && GetUserIndex() != INDEX_NONE)
    {
        FSlateApplication::Get().SetUserFocus(GetUserIndex(), Focusable, EFocusCause::SetDirectly);
    }
}

void UNsUISubsystem::RestoreFocus(UUserWidget* const Widget)
{
    if (Stack->GetActive() != nullptr && Stack->GetActive()->Widget == Widget)
    {
        ApplyFocus();
    }
}

void UNsUISubsystem::SetPointerMode(const ENsUIPointerMode NewPointerMode)
{
    if (PointerMode == NewPointerMode)
    {
        return;
    }
    PointerMode = NewPointerMode;
    ApplyInputPolicy();
    UpdateCursor();
    OnPointerModeChanged.Broadcast(PointerMode);
}

TArray<FKey> UNsUISubsystem::GetKeysForAction(const ENsUIAction Action) const
{
    return InputActions.IsValid() ? InputActions->GetKeys(Action) : TArray<FKey>();
}

FString UNsUISubsystem::DescribeState() const
{
    FString Text = FString::Printf(TEXT("NsUI user %d | pointer %s | suspended %s"), GetUserIndex(),
        PointerMode == ENsUIPointerMode::Mouse ? TEXT("Mouse") : TEXT("Keys"),
        FocusTracker.IsValid() && FocusTracker->IsSuspended() ? TEXT("yes") : TEXT("no"));
    if (InputActions.IsValid())
    {
        Text += FString::Printf(TEXT("\n  UI keys: %s"), *InputActions->DescribeKeys());
    }
    if (FSlateApplication::IsInitialized() && GetUserIndex() != INDEX_NONE)
    {
        const TSharedPtr<SWidget> Focus = FSlateApplication::Get().GetUserFocusedWidget(GetUserIndex());
        Text += FString::Printf(TEXT("\n  Focus: %s"), Focus.IsValid() ? *Focus->ToString() : TEXT("none"));
    }
    if (!Stack.IsValid())
    {
        return Text;
    }
    const FNsUIWidgetEntry* const Active = Stack->GetActive();
    for (int32 Index = Stack->GetEntries().Num() - 1; Index >= 0; --Index)
    {
        const FNsUIWidgetEntry& Entry = Stack->GetEntries()[Index];
        Text += FString::Printf(TEXT("\n  %s | %s | %s | %s%s | mode %s"), *GetNameSafe(Entry.Widget),
            *StaticEnum<ENsUILayer>()->GetNameStringByValue(static_cast<int64>(Entry.Config.Layer)),
            *StaticEnum<ENsUIInputPolicy>()->GetNameStringByValue(static_cast<int64>(Entry.Config.InputPolicy)),
            &Entry == Active ? TEXT("active") : TEXT("open"), Entry.bClosing ? TEXT(" closing") : TEXT(""),
            Entry.Mode != nullptr ? *Entry.Mode->DescribeSelection() : TEXT("none"));
    }
    return Text;
}

void UNsUISubsystem::EnsureRoot()
{
    if (Root.IsValid())
    {
        return;
    }
    UGameViewportClient* const Viewport = GetLocalPlayer()->ViewportClient;
    if (Viewport == nullptr)
    {
        UE_LOG(LogNsUI, Error, TEXT("The local player has no game viewport, so NsUI cannot show widgets"));
        return;
    }
    Root = SNew(SNsUIRoot)
        .Player(GetLocalPlayer())
        .OnFindAction(FNsUIFindAction::CreateUObject(this, &ThisClass::FindAction))
        .OnAction(FNsUIRouteAction::CreateUObject(this, &ThisClass::RouteAction))
        .OnNavigate(FNsUIRouteNavigate::CreateUObject(this, &ThisClass::RouteNavigate))
        .OnPointer(FNsUIRoutePointer::CreateUObject(this, &ThisClass::RoutePointer))
        .OnDevice(FNsUIReportDevice::CreateUObject(this, &ThisClass::HandleDeviceUsed))
        .OnRequestFocus(FNsUIRequestFocus::CreateUObject(this, &ThisClass::RetryFocus));
    Viewport->AddViewportWidgetForPlayer(GetLocalPlayer(), Root.ToSharedRef(), GetDefault<UNsUISettings>()->RootZOrder);
}

void UNsUISubsystem::RemoveRoot()
{
    UGameViewportClient* const Viewport = GetLocalPlayer() != nullptr ? GetLocalPlayer()->ViewportClient.Get() : nullptr;
    if (Root.IsValid() && Viewport != nullptr)
    {
        Viewport->RemoveViewportWidgetForPlayer(GetLocalPlayer(), Root.ToSharedRef());
    }
    Root.Reset();
}

void UNsUISubsystem::EnsureCursor()
{
    const UNsUISettings* const Settings = GetDefault<UNsUISettings>();
    if (Cursor != nullptr || Settings->CursorWidgetClass.IsNull())
    {
        return;
    }
    const TSubclassOf<UNsUICursorWidget> CursorClass = Settings->CursorWidgetClass.LoadSynchronous();
    Cursor = CursorClass != nullptr ? CreateWidget<UNsUICursorWidget>(GetPlayerController(), CursorClass) : nullptr;
    if (Cursor != nullptr)
    {
        Cursor->AddToPlayerScreen(Settings->CursorZOrder);
        Cursor->HideCursor();
    }
}

void UNsUISubsystem::RemoveEntry(UUserWidget* const Widget)
{
    const FNsUIWidgetEntry* const Entry = Stack->Find(Widget);
    const TSharedPtr<SWidget> Slot = Entry != nullptr ? Entry->Slot : nullptr;
    if (!Stack->Remove(Widget))
    {
        return;
    }
    if (Root.IsValid() && Slot.IsValid())
    {
        Root->RemoveWidget(Slot.ToSharedRef());
    }
    UE_LOG(LogNsUI, Verbose, TEXT("Close %s"), *GetNameSafe(Widget));
    if (Widget->Implements<UNsUIWidgetInterface>())
    {
        INsUIWidgetInterface::Execute_OnClosed(Widget);
    }
    UpdateActive(false);
}

void UNsUISubsystem::UpdateActive(const bool bForce)
{
    UUserWidget* Previous = nullptr;
    const bool bChanged = Stack->UpdateActive(Previous);
    FNsUIWidgetEntry* const Active = Stack->GetActive();
    for (const FNsUIWidgetEntry& Entry : Stack->GetEntries())
    {
        if (Entry.Slot.IsValid())
        {
            SNsUIRoot::SetSlotInteractive(Entry.Slot.ToSharedRef(), &Entry == Active);
        }
    }
    if (!bChanged && !bForce)
    {
        return;
    }

    if (bChanged)
    {
        if (Previous != nullptr && Previous->Implements<UNsUIWidgetInterface>())
        {
            INsUIWidgetInterface::Execute_OnDeactivated(Previous);
        }
        if (Active != nullptr && Active->Widget->Implements<UNsUIWidgetInterface>())
        {
            INsUIWidgetInterface::Execute_OnActivated(Active->Widget, Active->bWasActive);
        }
        if (Active != nullptr)
        {
            Active->bWasActive = true;
        }
        if (Active != nullptr && Active->Mode != nullptr)
        {
            Active->Mode->Activate();
        }
        UE_LOG(LogNsUI, Verbose, TEXT("Active widget: %s"), *GetNameSafe(Active != nullptr ? Active->Widget.Get() : nullptr));
    }

    const ENsUIInputPolicy Policy = Active != nullptr ? Active->Config.InputPolicy : ENsUIInputPolicy::Passive;
    const bool bTakesUIInput = Policy == ENsUIInputPolicy::UIOnly || Policy == ENsUIInputPolicy::UIAndGame;
    if (bTakesUIInput)
    {
        InputActions->InstallNavigation(Policy == ENsUIInputPolicy::UIOnly);
    }
    else
    {
        InputActions->RemoveNavigation();
    }
    if (Root.IsValid())
    {
        Root->SetCatchesPointer(bTakesUIInput && Active->Mode != nullptr);
        Root->SetTakesFocus(bTakesUIInput && (Active->Mode != nullptr || Active->Config.FocusPolicy == ENsUIFocusPolicy::Self));
    }
    UpdateGameFirstInput(Policy == ENsUIInputPolicy::GameFirst);
    ApplyInputPolicy();
    if (bTakesUIInput)
    {
        FocusTracker->RequestRestore();
    }
    UpdateCursor();

    if (bChanged)
    {
        OnActiveWidgetChanged.Broadcast(Active != nullptr ? Active->Widget.Get() : nullptr);
    }
}

void UNsUISubsystem::ApplyInputPolicy()
{
    APlayerController* const Player = GetPlayerController();
    if (Player == nullptr || !Stack.IsValid())
    {
        return;
    }
    const FNsUIWidgetEntry* const Active = Stack->GetActive();
    const ENsUIInputPolicy Policy = Active != nullptr ? Active->Config.InputPolicy : ENsUIInputPolicy::Passive;
    if (Policy == ENsUIInputPolicy::UIOnly)
    {
        FInputModeUIOnly Mode;
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Player->SetInputMode(Mode);
    }
    else if (Policy == ENsUIInputPolicy::UIAndGame)
    {
        FInputModeGameAndUI Mode;
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Mode.SetHideCursorDuringCapture(false);
        Player->SetInputMode(Mode);
    }
    else
    {
        Player->SetInputMode(FInputModeGameOnly());
    }

    const bool bTakesUIInput = Policy == ENsUIInputPolicy::UIOnly || Policy == ENsUIInputPolicy::UIAndGame;
    Player->SetShowMouseCursor(bTakesUIInput && Active->Config.bShowMouseCursor && PointerMode == ENsUIPointerMode::Mouse);
}

void UNsUISubsystem::UpdateCursor()
{
    if (Cursor == nullptr)
    {
        return;
    }
    const FNsUIWidgetEntry* const Active = Stack->GetActive();
    const bool bTakesUIInput = Active != nullptr && (Active->Config.InputPolicy == ENsUIInputPolicy::UIOnly || Active->Config.InputPolicy == ENsUIInputPolicy::UIAndGame);
    const bool bShow = bTakesUIInput && PointerMode == ENsUIPointerMode::Keys && !FocusTracker->IsSuspended() && Active->Config.FocusPolicy == ENsUIFocusPolicy::Children;
    if (!bShow)
    {
        Cursor->HideCursor();
        return;
    }
    if (Active->Mode != nullptr)
    {
        Cursor->FollowSelectionMode(Active->Mode);
        return;
    }
    const TSharedPtr<SWidget> Focus = Active->LastFocus.Pin();
    if (Focus.IsValid() && IsInside(Active->Slot, Focus))
    {
        Cursor->FollowSlateWidget(Focus);
        return;
    }
    Cursor->HideCursor();
}

void UNsUISubsystem::UpdateGameFirstInput(const bool bBind)
{
    APlayerController* const Player = GetPlayerController();
    if (!bBind)
    {
        if (GameFirstInput != nullptr && Player != nullptr)
        {
            Player->PopInputComponent(GameFirstInput);
        }
        GameFirstInput = nullptr;
        return;
    }
    if (GameFirstInput != nullptr || Player == nullptr)
    {
        return;
    }
    const UNsUISettings* const Settings = GetDefault<UNsUISettings>();
    GameFirstInput = NewObject<UEnhancedInputComponent>(Player, TEXT("NsUIGameFirstInput"));
    if (const UInputAction* const Confirm = Settings->GetInputAction(ENsUIAction::Confirm))
    {
        GameFirstInput->BindAction(Confirm, ETriggerEvent::Started, this, &ThisClass::HandleGameFirstConfirm);
    }
    if (const UInputAction* const Back = Settings->GetInputAction(ENsUIAction::Back))
    {
        GameFirstInput->BindAction(Back, ETriggerEvent::Started, this, &ThisClass::HandleGameFirstBack);
    }
    Player->PushInputComponent(GameFirstInput);
}

void UNsUISubsystem::ApplyFocus()
{
    FNsUIWidgetEntry* const Active = Stack.IsValid() ? Stack->GetActive() : nullptr;
    const int32 UserIndex = GetUserIndex();
    if (Active == nullptr || !Root.IsValid() || UserIndex == INDEX_NONE)
    {
        return;
    }
    if (FocusTracker->IsSuspended())
    {
        UE_LOG(LogNsUI, Verbose, TEXT("ApplyFocus waits: the game window has no focus"));
        return;
    }
    const ENsUIInputPolicy Policy = Active->Config.InputPolicy;
    if ((Policy != ENsUIInputPolicy::UIOnly && Policy != ENsUIInputPolicy::UIAndGame) || Root->IsConsoleOpen() || FSlateApplication::Get().AnyMenusVisible())
    {
        return;
    }

    FSlateApplication& Slate = FSlateApplication::Get();
    const TSharedPtr<SWidget> Current = Slate.GetUserFocusedWidget(UserIndex);
    if (Active->Config.FocusPolicy == ENsUIFocusPolicy::Self || Active->Mode != nullptr)
    {
        if (Current != Root)
        {
            Slate.SetUserFocus(UserIndex, Root, EFocusCause::SetDirectly);
        }
        return;
    }
    if (Current.IsValid() && IsInside(Active->Slot, Current))
    {
        return;
    }
    UE_LOG(LogNsUI, Verbose, TEXT("ApplyFocus for %s, the focus was %s"), *Active->Widget->GetName(), Current.IsValid() ? *Current->GetTypeAsString() : TEXT("none"));

    TSharedPtr<SWidget> Target = Active->LastFocus.Pin();
    if (!CanFocus(Target) || !IsInside(Active->Slot, Target))
    {
        const FNsUIFocusRequest Request = Active->Widget->Implements<UNsUIWidgetInterface>() ? INsUIWidgetInterface::Execute_GetFocus(Active->Widget) : FNsUIFocusRequest();
        const TSharedPtr<SWidget> Preferred = Request.PreferredTarget != nullptr ? Request.PreferredTarget->GetCachedWidget() : nullptr;
        Target = Preferred.IsValid() ? (CanFocus(Preferred) ? Preferred : FindFirstFocusable(Preferred.ToSharedRef())) : nullptr;
    }
    if (!CanFocus(Target) && Active->Slot.IsValid())
    {
        Target = FindFirstFocusable(Active->Slot.ToSharedRef());
    }
    // A control under a hidden or disabled panel cannot take the focus yet
    if (!CanFocus(Target))
    {
        Target.Reset();
    }
    // A widget with no control keeps the focus on the root, so its keys still reach the handlers
    if (!Target.IsValid())
    {
        UE_LOG(LogNsUI, Verbose, TEXT("%s has no control that can take the focus now. The root keeps it until a key or RestoreFocus"), *Active->Widget->GetName());
        Root->SetTakesFocus(true);
    }
    UE_LOG(LogNsUI, Verbose, TEXT("ApplyFocus gives the focus to %s"), Target.IsValid() ? *Target->GetTypeAsString() : TEXT("the root"));
    Slate.SetUserFocus(UserIndex, Target.IsValid() ? Target : StaticCastSharedPtr<SWidget>(Root), EFocusCause::SetDirectly);
}

bool UNsUISubsystem::RetryFocus()
{
    const FNsUIWidgetEntry* const Active = Stack->GetActive();
    if (Active == nullptr || Active->Mode != nullptr || Active->Config.FocusPolicy != ENsUIFocusPolicy::Children || GetUserIndex() == INDEX_NONE)
    {
        return false;
    }
    ApplyFocus();
    return FSlateApplication::Get().GetUserFocusedWidget(GetUserIndex()) != Root;
}

bool UNsUISubsystem::RouteAction(const ENsUIAction Action, const bool bPreview, const bool bGamepad)
{
    FNsUIWidgetEntry* const Active = Stack->GetActive();
    if (Active == nullptr || FocusTracker->IsSuspended())
    {
        return false;
    }
    UUserWidget* const Widget = Active->Widget;
    const bool bInterface = Widget->Implements<UNsUIWidgetInterface>();
    switch (Action)
    {
    case ENsUIAction::Confirm:
        if (bPreview)
        {
            return bInterface && INsUIWidgetInterface::Execute_HandleConfirm(Widget);
        }
        return Active->Mode != nullptr && Active->Mode->Confirm();
    case ENsUIAction::Back:
        if (bInterface && INsUIWidgetInterface::Execute_HandleBack(Widget))
        {
            return true;
        }
        if (Active->Mode != nullptr && Active->Mode->Cancel())
        {
            return true;
        }
        CloseWidget(Widget);
        return true;
    case ENsUIAction::NextTab:
    case ENsUIAction::PreviousTab:
        return bInterface && INsUIWidgetInterface::Execute_HandleTab(Widget, Action == ENsUIAction::NextTab);
    default:
        return false;
    }
}

bool UNsUISubsystem::RouteNavigate(const EUINavigation Direction)
{
    FNsUIWidgetEntry* const Active = Stack->GetActive();
    if (Active == nullptr || FocusTracker->IsSuspended())
    {
        return false;
    }
    if (Active->Widget->Implements<UNsUIWidgetInterface>() && INsUIWidgetInterface::Execute_HandleNavigate(Active->Widget, Direction))
    {
        return true;
    }
    if (Active->Mode != nullptr && Active->Mode->Navigate(Direction))
    {
        UpdateCursor();
        return true;
    }
    return false;
}

bool UNsUISubsystem::RoutePointer(const FPointerEvent& Event, const bool bPressed)
{
    FNsUIWidgetEntry* const Active = Stack->GetActive();
    if (Active == nullptr || FocusTracker->IsSuspended())
    {
        return false;
    }

    if (Active->Mode != nullptr)
    {
        FVector2D Pixels;
        FVector2D Position;
        USlateBlueprintLibrary::AbsoluteToViewport(GetLocalPlayer()->GetWorld(), Event.GetScreenSpacePosition(), Pixels, Position);
        const bool bPicked = Active->Mode->PickAt(Position);
        if (bPressed && bPicked && Event.GetEffectingButton() == EKeys::LeftMouseButton)
        {
            return (Active->Widget->Implements<UNsUIWidgetInterface>() && INsUIWidgetInterface::Execute_HandleConfirm(Active->Widget)) || Active->Mode->Confirm();
        }
        return false;
    }

    // Hover is focus: the control under the mouse takes the focus, so the keys and the mouse select the same control
    if (bPressed || Active->Config.FocusPolicy != ENsUIFocusPolicy::Children)
    {
        return false;
    }
    FSlateApplication& Slate = FSlateApplication::Get();
    const FWidgetPath Path = Slate.LocateWindowUnderMouse(Event.GetScreenSpacePosition(), Slate.GetInteractiveTopLevelWindows());
    for (int32 Index = Path.Widgets.Num() - 1; Index >= 0; --Index)
    {
        const TSharedRef<SWidget> Widget = Path.Widgets[Index].Widget;
        if (CanFocus(Widget) && IsInside(Active->Slot, Widget))
        {
            if (Slate.GetUserFocusedWidget(GetUserIndex()) != Widget)
            {
                Slate.SetUserFocus(GetUserIndex(), Widget, EFocusCause::Mouse);
            }
            break;
        }
    }
    return false;
}

TOptional<ENsUIAction> UNsUISubsystem::FindAction(const FKey& Key) const
{
    return InputActions.IsValid() ? InputActions->FindAction(Key) : TOptional<ENsUIAction>();
}

void UNsUISubsystem::HandleDeviceUsed(const ENsUIPointerMode Mode, const bool bGamepad)
{
    const bool bChanged = Mode != PointerMode || bGamepad != bLastDeviceGamepad;
    bLastDeviceGamepad = bGamepad;
    SetPointerMode(Mode);
    if (bChanged)
    {
        OnDeviceUsed.Broadcast(bGamepad);
    }
}

void UNsUISubsystem::HandleFocusMoved(const TSharedPtr<SWidget>& OldWidget, const TSharedPtr<SWidget>& NewWidget)
{
    FNsUIWidgetEntry* const Active = Stack->GetActive();
    if (Active == nullptr)
    {
        return;
    }
    if (NewWidget.IsValid() && IsInside(Active->Slot, NewWidget))
    {
        Active->LastFocus = NewWidget;
        UWidget* const OldFocus = IsInside(Active->Slot, OldWidget) ? FindUMGWidget(OldWidget) : nullptr;
        UWidget* const NewFocus = FindUMGWidget(NewWidget);
        if (OldFocus != NewFocus)
        {
            if (Active->Widget->Implements<UNsUIWidgetInterface>())
            {
                INsUIWidgetInterface::Execute_OnFocusChanged(Active->Widget, OldFocus, NewFocus);
            }
            OnFocusChanged.Broadcast(Active->Widget, OldFocus, NewFocus);
        }
        UpdateCursor();
        return;
    }
    if (NewWidget != Root)
    {
        FocusTracker->RequestRestore();
    }
}

void UNsUISubsystem::HandleSuspendedChanged(const bool bSuspended)
{
    if (!bSuspended)
    {
        FocusTracker->RequestRestore();
    }
    UpdateCursor();
}

void UNsUISubsystem::HandleWorldCleanup(UWorld* const World, bool, bool)
{
    TArray<UUserWidget*> Closing;
    for (const FNsUIWidgetEntry& Entry : Stack->GetEntries())
    {
        if (Entry.Widget != nullptr && Entry.Widget->GetWorld() == World)
        {
            Closing.Add(Entry.Widget);
        }
    }
    for (UUserWidget* const Widget : Closing)
    {
        RemoveEntry(Widget);
    }
    if (Cursor != nullptr && Cursor->GetWorld() == World)
    {
        Cursor->RemoveFromParent();
        Cursor = nullptr;
    }
    if (GameFirstInput != nullptr && GameFirstInput->GetWorld() == World)
    {
        GameFirstInput = nullptr;
    }
    // A map change removes all viewport widgets, so the next open makes a new root
    if (Stack->GetEntries().IsEmpty())
    {
        RemoveRoot();
    }
}

void UNsUISubsystem::HandleMappingsRebuilt()
{
    InputActions->ReadKeys();
}

void UNsUISubsystem::HandleGameFirstConfirm()
{
    const FNsUIWidgetEntry* const Active = Stack->GetActive();
    if (Active != nullptr && Active->Config.InputPolicy == ENsUIInputPolicy::GameFirst && Active->Widget->Implements<UNsUIWidgetInterface>())
    {
        INsUIWidgetInterface::Execute_HandleConfirm(Active->Widget);
    }
}

void UNsUISubsystem::HandleGameFirstBack()
{
    const FNsUIWidgetEntry* const Active = Stack->GetActive();
    if (Active != nullptr && Active->Config.InputPolicy == ENsUIInputPolicy::GameFirst && Active->Widget->Implements<UNsUIWidgetInterface>())
    {
        INsUIWidgetInterface::Execute_HandleBack(Active->Widget);
    }
}

APlayerController* UNsUISubsystem::GetPlayerController() const
{
    const ULocalPlayer* const Player = GetLocalPlayer();
    UWorld* const World = Player != nullptr ? Player->GetWorld() : nullptr;
    return World != nullptr && World->IsGameWorld() ? Player->GetPlayerController(World) : nullptr;
}

int32 UNsUISubsystem::GetUserIndex() const
{
    const ULocalPlayer* const Player = GetLocalPlayer();
    const TSharedPtr<const FSlateUser> User = Player != nullptr ? Player->GetSlateUser() : nullptr;
    return User.IsValid() ? User->GetUserIndex() : INDEX_NONE;
}

bool UNsUISubsystem::IsInside(const TSharedPtr<SWidget>& Ancestor, const TSharedPtr<SWidget>& Widget)
{
    if (!Ancestor.IsValid())
    {
        return false;
    }
    for (TSharedPtr<SWidget> Current = Widget; Current.IsValid(); Current = Current->GetParentWidget())
    {
        if (Current == Ancestor)
        {
            return true;
        }
    }
    return false;
}

TSharedPtr<SWidget> UNsUISubsystem::FindFirstFocusable(const TSharedRef<SWidget>& Widget)
{
    if (!Widget->GetVisibility().IsVisible() || !Widget->IsEnabled())
    {
        return nullptr;
    }
    if (Widget->SupportsKeyboardFocus())
    {
        return Widget;
    }
    FChildren* const Children = Widget->GetChildren();
    for (int32 Index = 0; Children != nullptr && Index < Children->Num(); ++Index)
    {
        if (const TSharedPtr<SWidget> Found = FindFirstFocusable(Children->GetChildAt(Index)))
        {
            return Found;
        }
    }
    return nullptr;
}

bool UNsUISubsystem::CanFocus(const TSharedPtr<SWidget>& Widget)
{
    if (!Widget.IsValid() || !Widget->SupportsKeyboardFocus() || !Widget->IsEnabled())
    {
        return false;
    }
    for (TSharedPtr<SWidget> Current = Widget; Current.IsValid(); Current = Current->GetParentWidget())
    {
        if (!Current->GetVisibility().IsVisible())
        {
            return false;
        }
    }
    return true;
}

UWidget* UNsUISubsystem::FindUMGWidget(const TSharedPtr<SWidget>& Widget)
{
    TArray<TSharedPtr<SWidget>> Chain;
    for (TSharedPtr<SWidget> Current = Widget; Current.IsValid(); Current = Current->GetParentWidget())
    {
        if (Current->GetType() == TEXT("SObjectWidget"))
        {
            UUserWidget* const Owner = StaticCastSharedPtr<SObjectWidget>(Current)->GetWidgetObject();
            UWidget* Found = nullptr;
            int32 FoundDepth = Chain.Num();
            if (Owner != nullptr && Owner->WidgetTree != nullptr)
            {
                // The nearest UMG widget to the focused Slate widget wins
                Owner->WidgetTree->ForEachWidget([&Chain, &Found, &FoundDepth](UWidget* const Candidate)
                {
                    const TSharedPtr<SWidget> Cached = Candidate != nullptr ? Candidate->GetCachedWidget() : nullptr;
                    const int32 Depth = Cached.IsValid() ? Chain.IndexOfByKey(Cached) : INDEX_NONE;
                    if (Depth != INDEX_NONE && Depth < FoundDepth)
                    {
                        Found = Candidate;
                        FoundDepth = Depth;
                    }
                });
            }
            return Found != nullptr ? Found : Owner;
        }
        Chain.Add(Current);
    }
    return nullptr;
}
