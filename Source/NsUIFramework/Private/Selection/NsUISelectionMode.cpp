#include "Selection/NsUISelectionMode.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NsUISettings.h"

void UNsUISelectionMode::Initialize(UUserWidget* const InWidget)
{
    Widget = InWidget;
}

APlayerController* UNsUISelectionMode::GetPlayerController() const
{
    return Widget.IsValid() ? Widget->GetOwningPlayer() : nullptr;
}

UNsUIWorldSelectionMode::UNsUIWorldSelectionMode()
{
    bSelectOnActivate = true;
    SelectedIndex = INDEX_NONE;
}

void UNsUIWorldSelectionMode::Activate()
{
    ClampSelection();
    if (SelectedIndex != INDEX_NONE || !bSelectOnActivate)
    {
        return;
    }

    // The first selection is the candidate nearest to the screen center
    const FVector2D Center = UWidgetLayoutLibrary::GetViewportSize(Widget.Get()) / FMath::Max(UWidgetLayoutLibrary::GetViewportScale(Widget.Get()), UE_SMALL_NUMBER) * 0.5;
    int32 Best = INDEX_NONE;
    double BestDistance = TNumericLimits<double>::Max();
    for (int32 Index = 0; Index < GetCandidateCount(); ++Index)
    {
        FVector2D Position;
        const double Distance = ProjectCandidate(Index, Position) ? FVector2D::DistSquared(Position, Center) : TNumericLimits<double>::Max();
        if (Distance < BestDistance)
        {
            Best = Index;
            BestDistance = Distance;
        }
    }
    SelectIndex(Best);
}

bool UNsUIWorldSelectionMode::Navigate(const EUINavigation Direction)
{
    ClampSelection();
    const int32 Count = GetCandidateCount();
    if (Count == 0)
    {
        return false;
    }
    if (Direction == EUINavigation::Next || Direction == EUINavigation::Previous)
    {
        const int32 Step = Direction == EUINavigation::Next ? 1 : -1;
        SelectIndex(SelectedIndex == INDEX_NONE ? 0 : (SelectedIndex + Step + Count) % Count);
        return true;
    }

    FVector2D From;
    if (SelectedIndex == INDEX_NONE || !ProjectCandidate(SelectedIndex, From))
    {
        Activate();
        return SelectedIndex != INDEX_NONE;
    }

    FVector2D Axis = FVector2D::ZeroVector;
    switch (Direction)
    {
    case EUINavigation::Up: Axis = FVector2D(0.0, -1.0); break;
    case EUINavigation::Down: Axis = FVector2D(0.0, 1.0); break;
    case EUINavigation::Left: Axis = FVector2D(-1.0, 0.0); break;
    case EUINavigation::Right: Axis = FVector2D(1.0, 0.0); break;
    default: return false;
    }

    // The next candidate is the nearest one inside the cone of the direction
    const double MinCos = FMath::Cos(FMath::DegreesToRadians(GetDefault<UNsUISettings>()->WorldNavigationConeDegrees));
    int32 Best = INDEX_NONE;
    double BestDistance = TNumericLimits<double>::Max();
    for (int32 Index = 0; Index < Count; ++Index)
    {
        FVector2D To;
        if (Index == SelectedIndex || !ProjectCandidate(Index, To))
        {
            continue;
        }
        const FVector2D Delta = To - From;
        const double Distance = Delta.Size();
        if (Distance > UE_KINDA_SMALL_NUMBER && FVector2D::DotProduct(Delta / Distance, Axis) >= MinCos && Distance < BestDistance)
        {
            Best = Index;
            BestDistance = Distance;
        }
    }
    if (Best != INDEX_NONE)
    {
        SelectIndex(Best);
    }
    // A direction with no candidate still belongs to the mode, so the focus does not leave the widget
    return true;
}

bool UNsUIWorldSelectionMode::PickAt(const FVector2D& ViewportPosition)
{
    const double Radius = GetDefault<UNsUISettings>()->PointerPickRadius;
    int32 Best = INDEX_NONE;
    double BestDistance = Radius * Radius;
    for (int32 Index = 0; Index < GetCandidateCount(); ++Index)
    {
        FVector2D Position;
        const double Distance = ProjectCandidate(Index, Position) ? FVector2D::DistSquared(Position, ViewportPosition) : TNumericLimits<double>::Max();
        if (Distance <= BestDistance)
        {
            Best = Index;
            BestDistance = Distance;
        }
    }
    if (Best != INDEX_NONE)
    {
        SelectIndex(Best);
    }
    return Best != INDEX_NONE;
}

bool UNsUIWorldSelectionMode::GetSelectionPosition(FVector2D& OutViewportPosition) const
{
    return SelectedIndex != INDEX_NONE && SelectedIndex < GetCandidateCount() && ProjectCandidate(SelectedIndex, OutViewportPosition);
}

void UNsUIWorldSelectionMode::SelectIndex(const int32 Index)
{
    const int32 Next = Index >= 0 && Index < GetCandidateCount() ? Index : INDEX_NONE;
    if (Next == SelectedIndex)
    {
        return;
    }
    SelectedIndex = Next;
    OnSelectionChanged.Broadcast(this);
}

bool UNsUIWorldSelectionMode::ProjectCandidate(const int32 Index, FVector2D& OutViewportPosition) const
{
    FVector World;
    APlayerController* const Player = GetPlayerController();
    if (Player == nullptr || !GetCandidatePosition(Index, World))
    {
        return false;
    }
    if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(Player, World, OutViewportPosition, false))
    {
        return false;
    }
    const FVector2D Size = UWidgetLayoutLibrary::GetViewportSize(Player) / FMath::Max(UWidgetLayoutLibrary::GetViewportScale(Player), UE_SMALL_NUMBER);
    return OutViewportPosition.X >= 0.0 && OutViewportPosition.Y >= 0.0 && OutViewportPosition.X <= Size.X && OutViewportPosition.Y <= Size.Y;
}

void UNsUIWorldSelectionMode::ClampSelection()
{
    if (SelectedIndex >= GetCandidateCount())
    {
        SelectIndex(INDEX_NONE);
    }
}

FString UNsUIActorSelectionMode::DescribeSelection() const
{
    return GetNameSafe(GetSelectedActor());
}

void UNsUIActorSelectionMode::SetCandidates(const TArray<AActor*>& InCandidates)
{
    Candidates.Reset(InCandidates.Num());
    for (AActor* const Actor : InCandidates)
    {
        if (Actor != nullptr)
        {
            Candidates.AddUnique(Actor);
        }
    }
    SelectedIndex = INDEX_NONE;
    Activate();
}

AActor* UNsUIActorSelectionMode::GetSelectedActor() const
{
    return Candidates.IsValidIndex(SelectedIndex) ? Candidates[SelectedIndex].Get() : nullptr;
}

bool UNsUIActorSelectionMode::GetCandidatePosition(const int32 Index, FVector& OutPosition) const
{
    const AActor* const Actor = Candidates.IsValidIndex(Index) ? Candidates[Index].Get() : nullptr;
    if (Actor == nullptr)
    {
        return false;
    }
    OutPosition = Actor->GetActorLocation();
    return true;
}

FString UNsUILocationSelectionMode::DescribeSelection() const
{
    return GetSelectedId().ToString();
}

void UNsUILocationSelectionMode::SetCandidates(const TArray<FNsUILocation>& InCandidates)
{
    Candidates.Reset(InCandidates.Num());
    TSet<FName> Ids;
    for (const FNsUILocation& Location : InCandidates)
    {
        bool bRepeated = false;
        Ids.Add(Location.Id, &bRepeated);
        if (!Location.Id.IsNone() && !bRepeated)
        {
            Candidates.Add(Location);
        }
    }
    SelectedIndex = INDEX_NONE;
    Activate();
}

FName UNsUILocationSelectionMode::GetSelectedId() const
{
    return Candidates.IsValidIndex(SelectedIndex) ? Candidates[SelectedIndex].Id : NAME_None;
}

bool UNsUILocationSelectionMode::GetCandidatePosition(const int32 Index, FVector& OutPosition) const
{
    if (!Candidates.IsValidIndex(Index))
    {
        return false;
    }
    OutPosition = Candidates[Index].Position;
    return true;
}
