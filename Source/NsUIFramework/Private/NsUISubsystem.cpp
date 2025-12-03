// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUISubsystem.h"

#include "Blueprint/WidgetTree.h"
#include "GameFramework/PlayerController.h"
#include "INsUINavigation.h"
#include "NsUICursor.h"
#include "NsUILayer.h"
#include "NsUILayout.h"
#include "NsUILibrary.h"
#include "NsUISettings.h"
#include "NsUIWidget.h"
#include "Components/WidgetNavigation.h"
#include "Framework/Application/NavigationConfig.h"
#include "Framework/Application/SlateApplication.h"
#include "Blueprint/UserWidget.h"
#include "Slate/SlateNavigationEnums.h"
#include "Slate/WidgetTransform.h"
#include "Widgets/SViewport.h"
#include "Engine/Engine.h"

UNsUISubsystem::UNsUISubsystem()
    : CurrentActorIndex(INDEX_NONE)
    , bActorSelectionActive(false)
    , bCursorEnabled(true)
{
}

void UNsUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    SetupNavigationConfig();
}

void UNsUISubsystem::Deinitialize()
{
    if (CursorWidget != nullptr)
    {
        CursorWidget->RemoveFromParent();
        CursorWidget = nullptr;
    }

    if (ActiveUILayout != nullptr)
    {
        ActiveUILayout->RemoveFromParent();
        ActiveUILayout = nullptr;
    }

    Super::Deinitialize();
}

UNsUISubsystem* UNsUISubsystem::GetSubsystem()
{
    if (GEngine != nullptr)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (const UWorld* World = Context.World())
            {
                if (World->IsGameWorld() || World->IsPlayInEditor())
                {
                    if (UNsUISubsystem* Subsystem = UGameInstance::GetSubsystem<UNsUISubsystem>(World->GetGameInstance()))
                    {
                        return Subsystem;
                    }
                }
            }
        }
    }
    return nullptr;
}

void UNsUISubsystem::Tick(float DeltaTime)
{
    UpdateActorCursor();
}

TStatId UNsUISubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UNsUISubsystem, STATGROUP_Tickables);
}

void UNsUISubsystem::SetupNavigationConfig()
{
    const UNsUISettings* Settings = UNsUISettings::GetSettings();
    if (Settings == nullptr)
    {
        return;
    }

    bCursorEnabled = Settings->bEnableRetroCursor;

    TSharedRef<FNavigationConfig> Navigation = MakeShared<FNavigationConfig>();
    Navigation->bKeyNavigation = Settings->bEnableKeyNavigation;
    Navigation->bTabNavigation = Settings->bEnableTabNavigation;
    Navigation->bAnalogNavigation = Settings->bEnableAnalogNavigation;

    if (Settings->AnalogHorizontalKey.IsValid())
    {
        Navigation->AnalogHorizontalKey = Settings->AnalogHorizontalKey;
    }
    if (Settings->AnalogVerticalKey.IsValid())
    {
        Navigation->AnalogVerticalKey = Settings->AnalogVerticalKey;
    }

    FSlateApplication::Get().SetNavigationConfig(Navigation);
}

void UNsUISubsystem::SetActiveUILayout(const TSoftClassPtr<UNsUILayout>& InLayoutClass)
{
    APlayerController* LocalPlayerController = nullptr;
    if (const UWorld* World = GetWorld())
    {
        LocalPlayerController = World->GetFirstPlayerController();
    }

    if (LocalPlayerController == nullptr || InLayoutClass.IsNull())
    {
        return;
    }

    if (ActiveUILayout != nullptr)
    {
        ActiveUILayout->RemoveFromParent();
        ActiveUILayout = nullptr;
    }

    ActiveUILayout = CreateWidget<UNsUILayout>(LocalPlayerController, InLayoutClass.LoadSynchronous());
    if (ActiveUILayout != nullptr)
    {
        ActiveUILayout->AddToViewport();

        if (bCursorEnabled)
        {
            const UNsUISettings* Settings = UNsUISettings::GetSettings();

            TSubclassOf<UNsUICursor> CursorClass = UNsUICursor::StaticClass();
            if (Settings && !Settings->CursorWidgetClass.IsNull())
            {
                CursorClass = Settings->CursorWidgetClass.LoadSynchronous();
            }

            if (CursorClass != nullptr)
            {
                CursorWidget = CreateWidget<UNsUICursor>(LocalPlayerController, CursorClass);
                if (CursorWidget)
                {
                    CursorWidget->AddToViewport(1000);
                }
            }
        }
    }
}

UUserWidget* UNsUISubsystem::PushWidget(TSubclassOf<UUserWidget> WidgetClass, ENsUILayer Layer)
{
    if (!WidgetClass)
    {
        return nullptr;
    }

    if (UNsUILayer* LayerPtr = FindLayer(Layer))
    {
        if (APlayerController* PC = LayerPtr->GetOwningPlayer())
        {
            UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
            if (Widget && PushWidgetInstance(Widget, Layer))
            {
                return Widget;
            }
        }
    }

    return nullptr;
}

bool UNsUISubsystem::PushWidgetInstance(UUserWidget* Widget, ENsUILayer Layer)
{
    if (UNsUILayer* LayerPtr = FindLayer(Layer))
    {
        LayerPtr->PushWidget(Widget, CurrentFocusedWidget.Get());
        return true;
    }
    return false;
}

void UNsUISubsystem::PopWidget(ENsUILayer Layer)
{
    if (UNsUILayer* LayerPtr = FindLayer(Layer))
    {
        LayerPtr->PopWidget();
    }
}

void UNsUISubsystem::PopToRoot(ENsUILayer Layer)
{
    if (UNsUILayer* LayerPtr = FindLayer(Layer))
    {
        LayerPtr->PopToRoot();
        LayerPtr->RestoreLastFocus();
    }
}

void UNsUISubsystem::Navigate(EUINavigation Direction)
{
    if (Direction == EUINavigation::Invalid)
    {
        return;
    }

    if (bActorSelectionActive)
    {
        if (ActorFocusTargets.Num() == 0)
        {
            return;
        }

        const int32 NextIndex = FindBestActorBySpatialSearch(CurrentActorIndex, Direction);
        if (NextIndex != CurrentActorIndex)
        {
            CurrentActorIndex = NextIndex;
            UpdateActorCursor();
        }
        return;
    }

    UWidget* Focused = CurrentFocusedWidget.Get();
    if (!IsValid(Focused))
    {
        return;
    }

    if (UWidget* Explicit = ResolveEngineNavigation(Focused, Direction))
    {
        MoveFocusToWidget(Explicit);
        return;
    }

    if (UWidget* Fallback = RunWidgetSpatialFallback(Focused, Direction))
    {
        MoveFocusToWidget(Fallback);
    }
}

void UNsUISubsystem::Confirm()
{
    if (bActorSelectionActive)
    {
        if (ActorFocusTargets.IsValidIndex(CurrentActorIndex))
        {
            OnActorConfirmed.Broadcast(ActorFocusTargets[CurrentActorIndex].Actor.Get());
        }
        ExitActorSelectionMode();
        return;
    }

    if (UWidget* Focused = CurrentFocusedWidget.Get())
    {
        if (Focused->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
        {
            INsUINavigation::Execute_HandleUIConfirm(Focused);
        }
    }
}

void UNsUISubsystem::Cancel()
{
    if (bActorSelectionActive)
    {
        if (ActorFocusTargets.IsValidIndex(CurrentActorIndex))
        {
            OnActorCancelled.Broadcast(ActorFocusTargets[CurrentActorIndex].Actor.Get());
        }
        ExitActorSelectionMode();
        return;
    }

    if (UWidget* Focused = CurrentFocusedWidget.Get())
    {
        if (Focused->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
        {
            INsUINavigation::Execute_HandleUICancel(Focused);
        }
    }
}

void UNsUISubsystem::EnterActorSelectionMode(const TArray<AActor*>& Actors)
{
    ActorFocusTargets.Reset();
    for (AActor* Actor : Actors)
    {
        if (IsValid(Actor))
        {
            FBox Bounds(ForceInit);
            Actor->GetActorBounds(false, Bounds.Origin, Bounds.BoxExtent);

            FVector Corners[8];
            Bounds.GetVertices(Corners);

            FBox2D ScreenBounds(ForceInit);
            if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
            {
                for (const FVector& Corner : Corners)
                {
                    FVector2D ScreenPos;
                    if (PC->ProjectWorldLocationToScreen(Corner, ScreenPos))
                    {
                        ScreenBounds += ScreenPos;
                    }
                }
            }

            FNsUIActorFocusInfo Info;
            Info.Actor = Actor;
            Info.ScreenBounds = ScreenBounds;
            ActorFocusTargets.Add(Info);
        }
    }

    if (ActorFocusTargets.Num() > 0)
    {
        FocusBeforeActorMode = CurrentFocusedWidget;
        bActorSelectionActive = true;
        CurrentActorIndex = 0;
        UpdateActorCursor();
    }
}

void UNsUISubsystem::ExitActorSelectionMode()
{
    bActorSelectionActive = false;
    CurrentActorIndex = INDEX_NONE;
    ActorFocusTargets.Reset();

    if (CursorWidget)
    {
        CursorWidget->MoveToWidget(nullptr);
    }

    if (FocusBeforeActorMode.IsValid())
    {
        MoveFocusToWidget(FocusBeforeActorMode.Get());
    }
    FocusBeforeActorMode.Reset();
}

void UNsUISubsystem::NotifyWidgetFocused(UWidget* Widget)
{
    if (Widget != nullptr)
    {
        CurrentFocusedWidget = Widget;
        if (CursorWidget)
        {
            CursorWidget->MoveToWidget(Widget);
        }
    }
}

void UNsUISubsystem::NotifyWidgetFocusLost(UWidget* Widget)
{
    if (CurrentFocusedWidget.Get() == Widget)
    {
        CurrentFocusedWidget = nullptr;
    }
}

UNsUILayer* UNsUISubsystem::FindLayer(ENsUILayer Layer) const
{
    return ActiveUILayout ? ActiveUILayout->GetLayer(Layer) : nullptr;
}

UNsUILayer* UNsUISubsystem::FindLayerForWidget(const UWidget* Widget) const
{
    if (!ActiveUILayout || Widget == nullptr)
    {
        return nullptr;
    }

    for (const auto& Pair : ActiveUILayout->GetRegisteredLayers())
    {
        if (const UNsUILayer* Layer = Pair.Value)
        {
            if (Widget->IsDescendantOf(const_cast<UNsUILayer*>(Layer)))
            {
                return Pair.Value;
            }
        }
    }

    return nullptr;
}

void UNsUISubsystem::MoveFocusToWidget(UWidget* TargetWidget)
{
    if (!IsValid(TargetWidget))
    {
        return;
    }

    if (APlayerController* PC = TargetWidget->GetOwningPlayer())
    {
        TargetWidget->SetUserFocus(PC);
        TargetWidget->SetKeyboardFocus();
        CurrentFocusedWidget = TargetWidget;

        if (CursorWidget)
        {
            CursorWidget->MoveToWidget(TargetWidget);
        }
    }
}

UWidget* UNsUISubsystem::ResolveEngineNavigation(UWidget* FromWidget, EUINavigation Direction) const
{
    if (!IsValid(FromWidget))
    {
        return nullptr;
    }

    if (UWidgetNavigation* Navigation = FromWidget->GetNavigation())
    {
        const FWidgetNavigationData* NavData = nullptr;
        switch (Direction)
        {
            case EUINavigation::Up:    NavData = &Navigation->Up; break;
            case EUINavigation::Down:  NavData = &Navigation->Down; break;
            case EUINavigation::Left:  NavData = &Navigation->Left; break;
            case EUINavigation::Right: NavData = &Navigation->Right; break;
            default: break;
        }

        if (NavData && NavData->Rule == EUINavigationRule::Explicit)
        {
            return NavData->Widget.Get();
        }
    }

    if (const UNsUIWidget* NsWidget = Cast<UNsUIWidget>(FromWidget))
    {
        if (UWidget* Neighbor = NsWidget->GetNeighbor(Direction))
        {
            return Neighbor;
        }
    }

    return nullptr;
}

UWidget* UNsUISubsystem::RunWidgetSpatialFallback(UWidget* FromWidget, EUINavigation Direction) const
{
    if (!IsValid(FromWidget))
    {
        return nullptr;
    }

    const UNsUILayer* Layer = FindLayerForWidget(FromWidget);
    if (Layer == nullptr)
    {
        return nullptr;
    }

    TArray<UWidget*> Candidates;
    UNsUILibrary::GatherAllNavigableDescendants(Cast<UUserWidget>(Layer->PeekWidget()), Candidates);

    const FVector2D FromCenter = FromWidget->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(0.5f, 0.5f));
    const FVector2D DesiredDirection = FVector2D(Direction == EUINavigation::Left ? -1.f : Direction == EUINavigation::Right ? 1.f : 0.f,
                                                 Direction == EUINavigation::Up ? -1.f : Direction == EUINavigation::Down ? 1.f : 0.f);

    UWidget* BestWidget = nullptr;
    float BestScore = -FLT_MAX;

    for (UWidget* Candidate : Candidates)
    {
        if (!IsValid(Candidate) || Candidate == FromWidget)
        {
            continue;
        }

        const FVector2D CandidateCenter = Candidate->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(0.5f, 0.5f));
        const FVector2D Delta = CandidateCenter - FromCenter;
        const float Dot = FVector2D::DotProduct(DesiredDirection, Delta.GetSafeNormal());

        if (Dot <= 0.f)
        {
            continue;
        }

        const float Score = Dot - 0.001f * Delta.SizeSquared();
        if (Score > BestScore)
        {
            BestScore = Score;
            BestWidget = Candidate;
        }
    }

    return BestWidget;
}

int32 UNsUISubsystem::FindBestActorBySpatialSearch(int32 CurrentIndex, EUINavigation NavDir) const
{
    if (ActorFocusTargets.Num() == 0)
    {
        return INDEX_NONE;
    }

    if (!ActorFocusTargets.IsValidIndex(CurrentIndex))
    {
        CurrentIndex = 0;
    }

    const FVector2D FromCenter = GetActorScreenCenter(ActorFocusTargets[CurrentIndex]);
    const FVector2D DesiredDirection = FVector2D(NavDir == EUINavigation::Left ? -1.f : NavDir == EUINavigation::Right ? 1.f : 0.f,
                                                 NavDir == EUINavigation::Up ? -1.f : NavDir == EUINavigation::Down ? 1.f : 0.f);

    int32 BestIndex = CurrentIndex;
    float BestScore = -FLT_MAX;

    for (int32 Index = 0; Index < ActorFocusTargets.Num(); ++Index)
    {
        if (Index == CurrentIndex)
        {
            continue;
        }

        const FVector2D CandidateCenter = GetActorScreenCenter(ActorFocusTargets[Index]);
        const FVector2D Delta = CandidateCenter - FromCenter;
        const float Dot = FVector2D::DotProduct(DesiredDirection, Delta.GetSafeNormal());

        if (Dot <= 0.f)
        {
            continue;
        }

        const float Score = Dot - 0.001f * Delta.SizeSquared();
        if (Score > BestScore)
        {
            BestScore = Score;
            BestIndex = Index;
        }
    }

    return BestIndex;
}

FVector2D UNsUISubsystem::GetActorScreenCenter(const FNsUIActorFocusInfo& Info) const
{
    return Info.ScreenBounds.GetCenter();
}

void UNsUISubsystem::UpdateActorCursor()
{
    if (!bActorSelectionActive || !CursorWidget)
    {
        return;
    }

    if (ActorFocusTargets.IsValidIndex(CurrentActorIndex))
    {
        CursorWidget->MoveToScreenPosition(GetActorScreenCenter(ActorFocusTargets[CurrentActorIndex]));
    }
    else
    {
        CursorWidget->MoveToWidget(nullptr);
    }
}
