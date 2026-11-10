#include "NsUIInputActions.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/NavigationConfig.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "NsUISettings.h"

FNsUIInputActions::FNsUIInputActions(ULocalPlayer* const InPlayer)
    : Player(InPlayer)
    , bInstalled(false)
    , bReportedMissingSettings(false)
{
}

FNsUIInputActions::~FNsUIInputActions()
{
    RemoveNavigation();
}

void FNsUIInputActions::AddMappingContext()
{
    const UNsUISettings* const Settings = GetDefault<UNsUISettings>();
    UEnhancedInputLocalPlayerSubsystem* const Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Player.Get());
    const UInputMappingContext* const Context = Settings->InputMappingContext.LoadSynchronous();
    if (Input == nullptr || Context == nullptr)
    {
        if (!bReportedMissingSettings)
        {
            bReportedMissingSettings = true;
            UE_LOG(LogNsUI, Error, TEXT("NsUI has no UI input. Set the Input Mapping Context and the Input Actions in Project Settings > Plugins > Ns UI Framework"));
        }
        return;
    }

    for (const TPair<ENsUIAction, TSoftObjectPtr<UInputAction>>& Pair : Settings->InputActions)
    {
        const UInputAction* const Action = Pair.Value.LoadSynchronous();
        if (Action != nullptr && Action->bConsumeInput)
        {
            UE_LOG(LogNsUI, Error, TEXT("The UI action %s consumes input, so the game loses its keys. Turn off Consume Input on the action"), *Action->GetName());
        }
    }

    if (!Input->HasMappingContext(Context))
    {
        Input->AddMappingContext(Context, Settings->InputMappingPriority);
    }
    ReadKeys();
}

void FNsUIInputActions::ReadKeys()
{
    const UEnhancedInputLocalPlayerSubsystem* const Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Player.Get());
    if (Input == nullptr)
    {
        return;
    }

    ActionOfKey.Reset();
    const UNsUISettings* const Settings = GetDefault<UNsUISettings>();
    for (const TPair<ENsUIAction, TSoftObjectPtr<UInputAction>>& Pair : Settings->InputActions)
    {
        const UInputAction* const Action = Pair.Value.LoadSynchronous();
        if (Action == nullptr)
        {
            continue;
        }
        for (const FKey& Key : Input->QueryKeysMappedToAction(Action))
        {
            ActionOfKey.Add(Key, Pair.Key);
        }
    }
    UE_LOG(LogNsUI, Log, TEXT("NsUI read %d UI keys from Enhanced Input"), ActionOfKey.Num());
    BuildNavigation();
}

TOptional<ENsUIAction> FNsUIInputActions::FindAction(const FKey& Key) const
{
    const ENsUIAction* const Found = ActionOfKey.Find(Key);
    return Found != nullptr ? TOptional<ENsUIAction>(*Found) : TOptional<ENsUIAction>();
}

TArray<FKey> FNsUIInputActions::GetKeys(const ENsUIAction Action) const
{
    TArray<FKey> Keys;
    for (const TPair<FKey, ENsUIAction>& Pair : ActionOfKey)
    {
        if (Pair.Value == Action)
        {
            Keys.Add(Pair.Key);
        }
    }
    return Keys;
}

FString FNsUIInputActions::DescribeKeys() const
{
    FString Text;
    for (const TPair<FKey, ENsUIAction>& Pair : ActionOfKey)
    {
        Text += FString::Printf(TEXT("%s=%s "), *Pair.Key.ToString(), *StaticEnum<ENsUIAction>()->GetNameStringByValue(static_cast<int64>(Pair.Value)));
    }
    return Text.IsEmpty() ? TEXT("none") : Text;
}

void FNsUIInputActions::InstallNavigation(const bool bAnalog)
{
    const TSharedPtr<FSlateUser> User = Player.IsValid() ? Player->GetSlateUser() : nullptr;
    if (!User.IsValid())
    {
        return;
    }
    if (!Navigation.IsValid())
    {
        BuildNavigation();
    }
    Navigation->bAnalogNavigation = bAnalog;
    if (!bInstalled)
    {
        PreviousNavigation = User->GetUserNavigationConfig();
        User->SetUserNavigationConfig(Navigation);
        bInstalled = true;
    }
}

void FNsUIInputActions::RemoveNavigation()
{
    if (!bInstalled)
    {
        return;
    }
    bInstalled = false;
    const TSharedPtr<FSlateUser> User = Player.IsValid() ? Player->GetSlateUser() : nullptr;
    if (User.IsValid() && User->GetUserNavigationConfig() == Navigation)
    {
        User->SetUserNavigationConfig(PreviousNavigation);
    }
    PreviousNavigation.Reset();
}

void FNsUIInputActions::BuildNavigation()
{
    const bool bAnalog = Navigation.IsValid() ? Navigation->bAnalogNavigation : true;
    // The engine default keys stay (arrows, D-pad, Enter, Escape), so the UI works also before Enhanced Input gives its keys. The UI actions add their keys
    TSharedRef<FNavigationConfig> Config = MakeShared<FNavigationConfig>();
    Config->bKeyNavigation = true;
    Config->bTabNavigation = true;
    Config->bAnalogNavigation = bAnalog;
    for (const TPair<FKey, ENsUIAction>& Pair : ActionOfKey)
    {
        switch (Pair.Value)
        {
        case ENsUIAction::Confirm: Config->KeyActionRules.Add(Pair.Key, EUINavigationAction::Accept); break;
        case ENsUIAction::Back: Config->KeyActionRules.Add(Pair.Key, EUINavigationAction::Back); break;
        case ENsUIAction::Up: Config->KeyEventRules.Add(Pair.Key, EUINavigation::Up); break;
        case ENsUIAction::Down: Config->KeyEventRules.Add(Pair.Key, EUINavigation::Down); break;
        case ENsUIAction::Left: Config->KeyEventRules.Add(Pair.Key, EUINavigation::Left); break;
        case ENsUIAction::Right: Config->KeyEventRules.Add(Pair.Key, EUINavigation::Right); break;
        default: break;
        }
    }

    const TSharedPtr<FSlateUser> User = Player.IsValid() ? Player->GetSlateUser() : nullptr;
    if (bInstalled && User.IsValid() && User->GetUserNavigationConfig() == Navigation)
    {
        User->SetUserNavigationConfig(Config);
    }
    Navigation = Config;
}
