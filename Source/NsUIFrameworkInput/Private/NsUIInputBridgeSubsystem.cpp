#include "NsUIInputBridgeSubsystem.h"

#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateUser.h"
#include "NsUISubsystem.h"
#include "Modules/ModuleManager.h"

#if WITH_NSINPUT
#include "Classes/NsInputSubsystem.h"
#endif

void UNsUIInputBridgeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    if (UNsUISubsystem* const UI = Collection.InitializeDependency<UNsUISubsystem>())
    {
        DeviceHandle = UI->OnDeviceUsed.AddUObject(this, &ThisClass::HandleDeviceUsed);
    }
}

void UNsUIInputBridgeSubsystem::Deinitialize()
{
    if (UNsUISubsystem* const UI = GetLocalPlayer() != nullptr ? GetLocalPlayer()->GetSubsystem<UNsUISubsystem>() : nullptr)
    {
        UI->OnDeviceUsed.Remove(DeviceHandle);
    }
    Super::Deinitialize();
}

void UNsUIInputBridgeSubsystem::HandleDeviceUsed(const bool bGamepad)
{
#if WITH_NSINPUT
    const ULocalPlayer* const Player = GetLocalPlayer();
    const TSharedPtr<const FSlateUser> User = Player != nullptr ? Player->GetSlateUser() : nullptr;
    UNsInputSubsystem* const Input = Player != nullptr ? Player->GetSubsystem<UNsInputSubsystem>() : nullptr;
    if (Input != nullptr && User.IsValid())
    {
        Input->NotifyInputDeviceUsed(User->GetUserIndex(), bGamepad);
    }
#endif
}

IMPLEMENT_MODULE(FDefaultModuleImpl, NsUIFrameworkInput)
