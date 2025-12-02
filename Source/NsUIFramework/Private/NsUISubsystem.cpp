// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "NsUICursor.h"
#include "NsUILayout.h"
#include "NsUILayer.h"
#include "NsUISettings.h"
#include "Stats/Stats.h"

UNsUISubsystem::UNsUISubsystem()
    : ActiveUILayout(nullptr)
    , RetroCursor(nullptr)
    , RetroCursorClass(nullptr)
    , ActorSelectionIndex(INDEX_NONE)
    , bCursorTracksActorSelection(true)
{
}

void UNsUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (const UNsUISettings* Settings = GetDefault<UNsUISettings>())
    {
        RetroCursorClass = Settings->CursorWidgetClass;
    }
}

void UNsUISubsystem::Deinitialize()
{
    DestroyActiveLayout();
    RetroCursor = nullptr;
    ActorSelectionCandidates.Empty();
    ActorSelectionIndex = INDEX_NONE;
    RecentStateEvents.Empty();

    Super::Deinitialize();
}

void UNsUISubsystem::Tick(float DeltaTime)
{
    UpdateRetroCursorFromActorSelection();
}

TStatId UNsUISubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UNsUISubsystem, STATGROUP_Tickables);
}

UNsUISubsystem* UNsUISubsystem::GetSubsystem()
{
    if (const UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UNsUISubsystem>();
        }
    }
    return nullptr;
}

UUserWidget* UNsUISubsystem::PushWidgetToLayer(const TSoftClassPtr<UUserWidget>& InWidget, ENsUILayerType InLayerType, const FNsUILayerTransitionRequest& Options)
{
    if (!ActiveUILayout)
    {
        return nullptr;
    }

    FNsUILayerTransitionRequest LocalOptions = Options;
    LocalOptions.LayerType = InLayerType;
    LocalOptions.WidgetClass = InWidget;

    if (UNsUIWidgetStackElement* StackElement = ActiveUILayout->PushContentToLayer(InLayerType, InWidget, LocalOptions))
    {
        StackElement->Layer = InLayerType;
        LogStateEvent(StackElement, ENsUILayerTransitionOp::Push);
        return StackElement->Widget.Get();
    }
    return nullptr;
}

UUserWidget* UNsUISubsystem::ReplaceTopWidgetOnLayer(const TSoftClassPtr<UUserWidget>& InWidget, ENsUILayerType InLayerType, const FNsUILayerTransitionRequest& Options)
{
    if (!ActiveUILayout)
    {
        return nullptr;
    }

    FNsUILayerTransitionRequest LocalOptions = Options;
    LocalOptions.LayerType = InLayerType;
    LocalOptions.WidgetClass = InWidget;

    if (UNsUIWidgetStackElement* StackElement = ActiveUILayout->ReplaceTopWidgetOnLayer(InLayerType, InWidget, LocalOptions))
    {
        StackElement->Layer = InLayerType;
        LogStateEvent(StackElement, ENsUILayerTransitionOp::ReplaceTop);
        return StackElement->Widget.Get();
    }
    return nullptr;
}

void UNsUISubsystem::PopWidgetFromLayer(ENsUILayerType InLayerType)
{
    if (ActiveUILayout)
    {
        if (UNsUIWidgetStackElement* Element = ActiveUILayout->PopContentFromLayer(InLayerType))
        {
            LogStateEvent(Element, ENsUILayerTransitionOp::Pop);
        }
    }
}

void UNsUISubsystem::UnwindLayer(ENsUILayerType InLayerType)
{
    if (ActiveUILayout)
    {
        const TArray<UNsUIWidgetStackElement*> Removed = ActiveUILayout->UnwindLayer(InLayerType);
        for (UNsUIWidgetStackElement* Element : Removed)
        {
            LogStateEvent(Element, ENsUILayerTransitionOp::Unwind);
        }
    }
}

void UNsUISubsystem::RemoveWidgetFromLayer(ENsUILayerType InLayerType, UUserWidget* InWidget)
{
    if (ActiveUILayout)
    {
        if (UNsUIWidgetStackElement* Element = ActiveUILayout->RemoveWidgetFromLayer(InLayerType, InWidget))
        {
            LogStateEvent(Element, ENsUILayerTransitionOp::Remove);
        }
    }
}

int32 UNsUISubsystem::RemoveWidgetsWithTag(ENsUILayerType InLayerType, FName Tag)
{
    int32 Count = 0;
    if (ActiveUILayout)
    {
        Count = ActiveUILayout->RemoveWidgetsWithTag(InLayerType, Tag);
    }
    return Count;
}

void UNsUISubsystem::RemoveWidgetByStableId(ENsUILayerType InLayerType, const FGuid& StableId)
{
    if (ActiveUILayout)
    {
        if (UNsUIWidgetStackElement* Element = ActiveUILayout->RemoveWidgetByStableId(InLayerType, StableId))
        {
            LogStateEvent(Element, ENsUILayerTransitionOp::Remove);
        }
    }
}

void UNsUISubsystem::ClearLayer(ENsUILayerType InLayerType)
{
    if (ActiveUILayout)
    {
        ActiveUILayout->ClearLayer(InLayerType);
    }
}

void UNsUISubsystem::ClearAllLayers()
{
    if (ActiveUILayout)
    {
        ActiveUILayout->ClearAllLayers();
    }
}

UNsUIWidgetStackElement* UNsUISubsystem::ApplyLayerTransition(const FNsUILayerTransitionRequest& Request)
{
    if (!ActiveUILayout)
    {
        return nullptr;
    }

    FNsUILayerTransitionRequest LocalRequest = Request;

    switch (LocalRequest.Operation)
    {
    case ENsUILayerTransitionOp::Push:
        if (UNsUIWidgetStackElement* Element = ActiveUILayout->PushContentToLayer(LocalRequest.LayerType, LocalRequest.WidgetClass, LocalRequest))
        {
            Element->Layer = LocalRequest.LayerType;
            LogStateEvent(Element, ENsUILayerTransitionOp::Push);
            return Element;
        }
        return nullptr;
    case ENsUILayerTransitionOp::ReplaceTop:
        if (UNsUIWidgetStackElement* Element = ActiveUILayout->ReplaceTopWidgetOnLayer(LocalRequest.LayerType, LocalRequest.WidgetClass, LocalRequest))
        {
            Element->Layer = LocalRequest.LayerType;
            LogStateEvent(Element, ENsUILayerTransitionOp::ReplaceTop);
            return Element;
        }
        return nullptr;
    case ENsUILayerTransitionOp::Pop:
        if (UNsUIWidgetStackElement* Element = ActiveUILayout->PopContentFromLayer(LocalRequest.LayerType))
        {
            LogStateEvent(Element, ENsUILayerTransitionOp::Pop);
            return Element;
        }
        return nullptr;
    case ENsUILayerTransitionOp::Remove:
        if (UNsUIWidgetStackElement* Element = ActiveUILayout->RemoveWidgetFromLayer(LocalRequest.LayerType, LocalRequest.WidgetInstance))
        {
            LogStateEvent(Element, ENsUILayerTransitionOp::Remove);
            return Element;
        }
        return nullptr;
    case ENsUILayerTransitionOp::ClearLayer:
        ActiveUILayout->ClearLayer(LocalRequest.LayerType);
        return nullptr;
    case ENsUILayerTransitionOp::Unwind:
        ActiveUILayout->UnwindLayer(LocalRequest.LayerType);
        return nullptr;
    case ENsUILayerTransitionOp::ClearAll:
        ActiveUILayout->ClearAllLayers();
        return nullptr;
    default:
        break;
    }

    return nullptr;
}

void UNsUISubsystem::DestroyActiveLayout()
{
    if (ActiveUILayout)
    {
        ActiveUILayout->RemoveFromParent();
        ActiveUILayout = nullptr;
    }
}

void UNsUISubsystem::SetActiveUILayout(const TSoftClassPtr<UNsUILayout>& InLayout)
{
    if (!GetWorld())
    {
        return;
    }

    UClass* LayoutClass = InLayout.LoadSynchronous();
    if (!LayoutClass)
    {
        DestroyActiveLayout();
        return;
    }

    if (ActiveUILayout && ActiveUILayout->GetClass() == LayoutClass)
    {
        return;
    }

    DestroyActiveLayout();

    ActiveUILayout = CreateWidget<UNsUILayout>(GetWorld(), LayoutClass);
    if (ActiveUILayout)
    {
        ActiveUILayout->AddToViewport();
    }
}

UNsUILayout* UNsUISubsystem::GetActiveUILayout() const
{
    return ActiveUILayout;
}

void UNsUISubsystem::SetRetroCursorClass(const TSoftClassPtr<UNsUICursor>& InCursorClass)
{
    RetroCursorClass = InCursorClass;
    if (RetroCursor)
    {
        RetroCursor->RemoveFromParent();
        RetroCursor = nullptr;
    }
}

UNsUICursor* UNsUISubsystem::EnsureRetroCursor()
{
    if (!RetroCursor)
    {
        UClass* CursorClass = RetroCursorClass.IsNull() ? UNsUICursor::StaticClass() : RetroCursorClass.LoadSynchronous();
        if (!CursorClass)
        {
            return nullptr;
        }

        if (UWorld* World = GetWorld())
        {
            RetroCursor = CreateWidget<UNsUICursor>(World, CursorClass);
            if (RetroCursor)
            {
                RetroCursor->AddToViewport(10000);
            }
        }
    }
    return RetroCursor;
}

void UNsUISubsystem::MoveRetroCursorToWidget(UWidget* TargetWidget)
{
    if (UNsUICursor* Cursor = EnsureRetroCursor())
    {
        Cursor->MoveToWidget(TargetWidget);
    }
}

void UNsUISubsystem::MoveRetroCursorToScreenPosition(const FVector2D& ScreenPosition)
{
    if (UNsUICursor* Cursor = EnsureRetroCursor())
    {
        Cursor->MoveToScreenPosition(ScreenPosition);
    }
}

void UNsUISubsystem::StartActorSelectionSession(const TArray<TSoftObjectPtr<AActor>>& Actors, bool bSnapCursorToSelection)
{
    ActorSelectionCandidates.Empty();
    ActorSelectionIndex = INDEX_NONE;

    for (const TSoftObjectPtr<AActor>& ActorPtr : Actors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            ActorSelectionCandidates.Add(Actor);
        }
        else if (AActor* LoadedActor = ActorPtr.LoadSynchronous())
        {
            ActorSelectionCandidates.Add(LoadedActor);
        }
    }

    if (ActorSelectionCandidates.Num() > 0)
    {
        ActorSelectionIndex = 0;
        if (bSnapCursorToSelection)
        {
            UpdateRetroCursorFromActorSelection();
        }
    }
}

void UNsUISubsystem::ExitActorSelectionSession()
{
    ActorSelectionCandidates.Empty();
    ActorSelectionIndex = INDEX_NONE;
}

AActor* UNsUISubsystem::GetCurrentActorSelection() const
{
    return ActorSelectionCandidates.IsValidIndex(ActorSelectionIndex)
        ? ActorSelectionCandidates[ActorSelectionIndex].Get()
        : nullptr;
}

AActor* UNsUISubsystem::StepActorSelection(int32 Direction, bool bSnapCursorToSelection)
{
    if (ActorSelectionCandidates.Num() == 0)
    {
        return nullptr;
    }

    if (!ActorSelectionCandidates.IsValidIndex(ActorSelectionIndex))
    {
        ActorSelectionIndex = 0;
    }
    else
    {
        ActorSelectionIndex = (ActorSelectionIndex + Direction) % ActorSelectionCandidates.Num();
        if (ActorSelectionIndex < 0)
        {
            ActorSelectionIndex += ActorSelectionCandidates.Num();
        }
    }

    if (bSnapCursorToSelection)
    {
        UpdateRetroCursorFromActorSelection();
    }

    return GetCurrentActorSelection();
}

bool UNsUISubsystem::ProjectActorToScreen(const AActor* Actor, FVector2D& OutScreenPos) const
{
    OutScreenPos = FVector2D::ZeroVector;

    if (!Actor)
    {
        return false;
    }

    if (const UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return PC->ProjectWorldLocationToScreen(Actor->GetActorLocation(), OutScreenPos);
        }
    }

    return false;
}

void UNsUISubsystem::UpdateRetroCursorFromActorSelection()
{
    if (!bCursorTracksActorSelection)
    {
        return;
    }

    if (UNsUICursor* Cursor = EnsureRetroCursor())
    {
        if (AActor* Actor = GetCurrentActorSelection())
        {
            FVector2D ScreenPos;
            if (ProjectActorToScreen(Actor, ScreenPos))
            {
                Cursor->MoveToScreenPosition(ScreenPos);
                return;
            }
        }

        Cursor->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UNsUISubsystem::LogStateEvent(const UNsUIWidgetStackElement* Element, ENsUILayerTransitionOp Op)
{
    FNsUIStateEvent Event;
    Event.Layer = Element ? Element->Layer : ENsUILayerType::None;
    Event.Operation = Op;
    Event.Widget = Element ? Element->Widget : nullptr;
    Event.StableId = Element ? Element->StableId : FGuid();
    Event.Tags = Element ? Element->Tags : TArray<FName>();

    RecentStateEvents.Add(Event);
    const int32 MaxEvents = 32;
    while (RecentStateEvents.Num() > MaxEvents)
    {
        RecentStateEvents.RemoveAt(0);
    }
}
