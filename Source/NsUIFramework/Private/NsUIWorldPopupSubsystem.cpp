#include "NsUIWorldPopupSubsystem.h"

#include "NsUIWorldPopupWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

void UNsUIWorldPopupSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    CleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::OnWorldCleanup);
}

void UNsUIWorldPopupSubsystem::Deinitialize()
{
    FWorldDelegates::OnWorldCleanup.Remove(CleanupHandle);
    ClearPopups();
    Pool.Reset();
    Super::Deinitialize();
}

TStatId UNsUIWorldPopupSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UNsUIWorldPopupSubsystem, STATGROUP_Tickables);
}

UNsUIWorldPopupWidget* UNsUIWorldPopupSubsystem::ShowPopup(const TSubclassOf<UNsUIWorldPopupWidget> PopupClass, const FText& Text, const FVector Location, const FLinearColor Color, AActor* const FollowActor)
{
    const UWorld* World = GetWorld();
    APlayerController* const Player = GetLocalPlayer() && World ? GetLocalPlayer()->GetPlayerController(World) : nullptr;
    if (!PopupClass || !Player)
    {
        return nullptr;
    }

    // Over the limit of the class, the oldest popup of the class ends
    const int32 MaxActive = PopupClass->GetDefaultObject<UNsUIWorldPopupWidget>()->GetMaxActive();
    int32 ClassCount = 0;
    for (const FNsUIWorldPopupEntry& Entry : Active)
    {
        ClassCount += Entry.Widget && Entry.Widget->GetClass() == PopupClass ? 1 : 0;
    }
    for (int32 Index = 0; Index < Active.Num() && ClassCount >= MaxActive; )
    {
        if (Active[Index].Widget && Active[Index].Widget->GetClass() == PopupClass)
        {
            Release(Index);
            --ClassCount;
            continue;
        }
        ++Index;
    }

    UNsUIWorldPopupWidget* Widget = nullptr;
    const int32 PoolIndex = Pool.IndexOfByPredicate([PopupClass, Player](const UNsUIWorldPopupWidget* Pooled)
    {
        return IsValid(Pooled) && Pooled->GetClass() == PopupClass && Pooled->GetOwningPlayer() == Player;
    });
    if (PoolIndex != INDEX_NONE)
    {
        Widget = Pool[PoolIndex];
        Pool.RemoveAtSwap(PoolIndex);
    }
    else
    {
        Widget = CreateWidget<UNsUIWorldPopupWidget>(Player, PopupClass);
    }
    if (!Widget)
    {
        return nullptr;
    }

    Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
    Widget->SetAlignmentInViewport(FVector2D(0.5f, 1.f));
    Widget->AddToPlayerScreen(Widget->GetViewportZOrder());
    Widget->StartPopup(Text, Color);

    // A popup on an actor that has popups already starts higher, so the texts stack
    int32 Stacked = 0;
    for (const FNsUIWorldPopupEntry& Other : Active)
    {
        Stacked += FollowActor && Other.FollowActor.Get() == FollowActor ? 1 : 0;
    }

    FNsUIWorldPopupEntry& Entry = Active.AddDefaulted_GetRef();
    Entry.Widget = Widget;
    Entry.FollowActor = FollowActor;
    Entry.bFollows = FollowActor != nullptr;
    Entry.Location = Location + FVector::UpVector * Widget->GetStackHeight() * Stacked;
    Entry.Age = 0.f;
    UpdatePopup(Entry, Player, 0.f);
    return Widget;
}

void UNsUIWorldPopupSubsystem::ClearPopups()
{
    while (!Active.IsEmpty())
    {
        Release(Active.Num() - 1);
    }
}

void UNsUIWorldPopupSubsystem::Tick(const float DeltaTime)
{
    const UWorld* World = GetWorld();
    const APlayerController* Player = GetLocalPlayer() && World ? GetLocalPlayer()->GetPlayerController(World) : nullptr;
    for (int32 Index = Active.Num() - 1; Index >= 0; --Index)
    {
        if (!Player || !UpdatePopup(Active[Index], Player, DeltaTime))
        {
            Release(Index);
        }
    }
}

bool UNsUIWorldPopupSubsystem::UpdatePopup(FNsUIWorldPopupEntry& Entry, const APlayerController* const Player, const float DeltaTime) const
{
    UNsUIWorldPopupWidget* const Widget = Entry.Widget;
    const AActor* const FollowActor = Entry.FollowActor.Get();
    if (!IsValid(Widget) || Widget->GetOwningPlayer() != Player || (Entry.bFollows && !FollowActor))
    {
        return false;
    }

    Entry.Age += DeltaTime;
    if (!Widget->ApplyAge(Entry.Age))
    {
        return false;
    }

    const FVector Base = FollowActor ? FollowActor->GetActorLocation() + Entry.Location : Entry.Location;
    const FVector Point = Base + FVector::UpVector * Widget->GetRiseSpeed() * Entry.Age;
    const FVector CameraLocation = Player->PlayerCameraManager ? Player->PlayerCameraManager->GetCameraLocation() : Point;
    bool bShow = FVector::DistSquared(CameraLocation, Point) <= FMath::Square(Widget->GetMaxDistance());

    FVector2D ScreenPosition = FVector2D::ZeroVector;
    bShow = bShow && Player->ProjectWorldLocationToScreen(Point, ScreenPosition, true);
    if (bShow && Widget->HidesWhenBlocked())
    {
        FCollisionQueryParams Params(SCENE_QUERY_STAT(NsUIWorldPopup), false);
        Params.AddIgnoredActor(Player->GetPawn());
        Params.AddIgnoredActor(FollowActor);
        bShow = !Player->GetWorld()->LineTraceTestByChannel(CameraLocation, Point, ECC_Visibility, Params);
    }

    Widget->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
    if (bShow)
    {
        Widget->SetPositionInViewport(ScreenPosition, true);
    }
    return true;
}

void UNsUIWorldPopupSubsystem::Release(const int32 Index)
{
    UNsUIWorldPopupWidget* const Widget = Active[Index].Widget;
    Active.RemoveAt(Index);
    if (IsValid(Widget))
    {
        Widget->RemoveFromParent();
        Pool.Add(Widget);
    }
}

void UNsUIWorldPopupSubsystem::OnWorldCleanup(UWorld* World, bool, bool)
{
    for (int32 Index = Active.Num() - 1; Index >= 0; --Index)
    {
        if (!Active[Index].Widget || Active[Index].Widget->GetWorld() == World)
        {
            Release(Index);
        }
    }
    Pool.RemoveAll([World](const UNsUIWorldPopupWidget* Pooled) { return !IsValid(Pooled) || Pooled->GetWorld() == World; });
}
