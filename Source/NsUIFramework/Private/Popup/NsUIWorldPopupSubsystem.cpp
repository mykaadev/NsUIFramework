#include "Popup/NsUIWorldPopupSubsystem.h"

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "NsUIFrameworkTypes.h"
#include "Popup/NsUIWorldPopupWidget.h"

void UNsUIWorldPopupSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::HandleWorldCleanup);
}

void UNsUIWorldPopupSubsystem::Deinitialize()
{
    FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
    ClearPopups();
    Pool.Reset();
    Super::Deinitialize();
}

TStatId UNsUIWorldPopupSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UNsUIWorldPopupSubsystem, STATGROUP_Tickables);
}

UWorld* UNsUIWorldPopupSubsystem::GetTickableGameObjectWorld() const
{
    const ULocalPlayer* const Player = GetLocalPlayer();
    return Player != nullptr ? Player->GetWorld() : nullptr;
}

void UNsUIWorldPopupSubsystem::ShowPopup(const TSubclassOf<UNsUIWorldPopupWidget> PopupClass, const FText& Text, const FVector Location, const FLinearColor Color, AActor* const FollowActor)
{
    APlayerController* const Player = GetLocalPlayer() != nullptr ? GetLocalPlayer()->GetPlayerController(GetTickableGameObjectWorld()) : nullptr;
    if (PopupClass == nullptr || Player == nullptr)
    {
        UE_LOG(LogNsUI, Warning, TEXT("ShowPopup needs a popup class and a player controller"));
        return;
    }

    // Over the limit of the class, the oldest popup of the class ends
    const int32 MaxActive = GetDefault<UNsUIWorldPopupWidget>(PopupClass)->MaxActive;
    int32 SameClass = 0;
    for (const FNsUIWorldPopupEntry& Entry : Active)
    {
        SameClass += Entry.Widget != nullptr && Entry.Widget->GetClass() == PopupClass ? 1 : 0;
    }
    for (int32 Index = 0; Index < Active.Num() && SameClass >= MaxActive; )
    {
        if (Active[Index].Widget != nullptr && Active[Index].Widget->GetClass() == PopupClass)
        {
            Release(Index);
            --SameClass;
            continue;
        }
        ++Index;
    }

    const int32 PoolIndex = Pool.IndexOfByPredicate([PopupClass, Player](const UNsUIWorldPopupWidget* const Widget)
    {
        return Widget != nullptr && Widget->GetClass() == PopupClass && Widget->GetOwningPlayer() == Player;
    });
    UNsUIWorldPopupWidget* Widget = nullptr;
    if (PoolIndex != INDEX_NONE)
    {
        Widget = Pool[PoolIndex];
        Pool.RemoveAtSwap(PoolIndex);
    }
    else
    {
        Widget = CreateWidget<UNsUIWorldPopupWidget>(Player, PopupClass);
    }
    if (Widget == nullptr)
    {
        UE_LOG(LogNsUI, Error, TEXT("ShowPopup could not make a popup of %s"), *PopupClass->GetName());
        return;
    }

    int32 Stacked = 0;
    for (const FNsUIWorldPopupEntry& Entry : Active)
    {
        Stacked += FollowActor != nullptr && Entry.FollowActor == FollowActor ? 1 : 0;
    }

    FNsUIWorldPopupEntry& Entry = Active.AddDefaulted_GetRef();
    Entry.Widget = Widget;
    Entry.FollowActor = FollowActor;
    Entry.Point = Location + FVector::UpVector * (Widget->StackHeight * Stacked);
    Widget->AddToPlayerScreen(Widget->ViewportZOrder);
    Widget->SetAlignmentInViewport(Widget->ScreenAlignment);
    Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
    Widget->StartPopup(Text, Color);
}

void UNsUIWorldPopupSubsystem::ClearPopups()
{
    while (Active.Num() > 0)
    {
        Release(Active.Num() - 1);
    }
}

void UNsUIWorldPopupSubsystem::Tick(const float DeltaTime)
{
    const APlayerController* const Player = GetLocalPlayer() != nullptr ? GetLocalPlayer()->GetPlayerController(GetTickableGameObjectWorld()) : nullptr;
    for (int32 Index = Active.Num() - 1; Index >= 0; --Index)
    {
        FNsUIWorldPopupEntry& Entry = Active[Index];
        UNsUIWorldPopupWidget* const Widget = Entry.Widget;
        const bool bLostActor = !Entry.FollowActor.IsExplicitlyNull() && !Entry.FollowActor.IsValid();
        Entry.Age += DeltaTime;
        if (Widget == nullptr || Player == nullptr || bLostActor || Entry.Age >= Widget->Lifetime)
        {
            Release(Index);
            continue;
        }
        if (!Entry.bDisappearing && Entry.Age >= Widget->Lifetime - Widget->GetDisappearSeconds())
        {
            Entry.bDisappearing = true;
            Widget->PlayDisappear();
        }
        FVector2D ScreenPosition;
        const bool bVisible = UpdatePopup(Entry, Player, ScreenPosition);
        Widget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
        if (bVisible)
        {
            Widget->SetPositionInViewport(ScreenPosition, true);
        }
    }
}

bool UNsUIWorldPopupSubsystem::UpdatePopup(const FNsUIWorldPopupEntry& Entry, const APlayerController* const Player, FVector2D& OutScreenPosition) const
{
    const UNsUIWorldPopupWidget* const Widget = Entry.Widget;
    const AActor* const Actor = Entry.FollowActor.Get();
    const FVector World = (Actor != nullptr ? Actor->GetActorLocation() + Entry.Point : Entry.Point) + FVector::UpVector * (Widget->RiseSpeed * Entry.Age);

    FVector CameraLocation;
    FRotator CameraRotation;
    Player->GetPlayerViewPoint(CameraLocation, CameraRotation);
    if (FVector::DistSquared(CameraLocation, World) > FMath::Square(Widget->MaxDistance))
    {
        return false;
    }
    if (!Player->ProjectWorldLocationToScreen(World, OutScreenPosition, true))
    {
        return false;
    }
    if (!Widget->bHideWhenBlocked)
    {
        return true;
    }
    FCollisionQueryParams Params(SCENE_QUERY_STAT(NsUIWorldPopup), false);
    Params.AddIgnoredActor(Player->GetPawn());
    Params.AddIgnoredActor(Actor);
    return !Player->GetWorld()->LineTraceTestByChannel(CameraLocation, World, ECC_Visibility, Params);
}

void UNsUIWorldPopupSubsystem::Release(const int32 Index)
{
    UNsUIWorldPopupWidget* const Widget = Active[Index].Widget;
    Active.RemoveAt(Index);
    if (Widget != nullptr)
    {
        Widget->RemoveFromParent();
        Pool.Add(Widget);
    }
}

void UNsUIWorldPopupSubsystem::HandleWorldCleanup(UWorld* const World, bool, bool)
{
    for (int32 Index = Active.Num() - 1; Index >= 0; --Index)
    {
        if (Active[Index].Widget == nullptr || Active[Index].Widget->GetWorld() == World)
        {
            Release(Index);
        }
    }
    Pool.RemoveAll([World](const UNsUIWorldPopupWidget* const Widget) { return Widget == nullptr || Widget->GetWorld() == World; });
}
