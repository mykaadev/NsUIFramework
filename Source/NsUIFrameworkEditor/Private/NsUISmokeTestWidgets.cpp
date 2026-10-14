#include "Smoke/NsUISmokeTestWidgets.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/LocalPlayer.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"
#include "NsUISubsystem.h"
#include "Selection/NsUISelectionMode.h"

TSharedRef<SWidget> UNsUISmokeTestRootWidget::RebuildWidget()
{
    if (!WidgetTree->RootWidget)
    {
        UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
        Controls = WidgetTree->ConstructWidget<UVerticalBox>();
        UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Controls);
        PanelSlot->SetPosition(FVector2D(100, 100));
        PanelSlot->SetSize(FVector2D(300, 500));
        WidgetTree->RootWidget = Canvas;
        const auto Add = [&](const TCHAR* Label, FName Function)
        {
            UButton* Button = WidgetTree->ConstructWidget<UButton>();
            UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
            Text->SetText(FText::FromString(Label));
            Button->SetContent(Text);
            FScriptDelegate Callback;
            Callback.BindUFunction(this, Function);
            Button->OnClicked.Add(Callback);
            Controls->AddChildToVerticalBox(Button);
        };
        Add(TEXT("Menu"), GET_FUNCTION_NAME_CHECKED(ThisClass, OpenMenu));
        Add(TEXT("Modal"), GET_FUNCTION_NAME_CHECKED(ThisClass, OpenModal));
        Add(TEXT("Viewport screen"), GET_FUNCTION_NAME_CHECKED(ThisClass, OpenExternal));
        Add(TEXT("Add control"), GET_FUNCTION_NAME_CHECKED(ThisClass, AddControl));
        Add(TEXT("Remove control"), GET_FUNCTION_NAME_CHECKED(ThisClass, RemoveControl));
        Add(TEXT("Actors"), GET_FUNCTION_NAME_CHECKED(ThisClass, OpenActors));
        Add(TEXT("Locations"), GET_FUNCTION_NAME_CHECKED(ThisClass, OpenLocations));
        Add(TEXT("Close"), GET_FUNCTION_NAME_CHECKED(ThisClass, Close));
    }
    return Super::RebuildWidget();
}

void UNsUISmokeTestRootWidget::Close()
{
    if (IsInViewport()) { RemoveFromParent(); }
    else { GetOwningLocalPlayer()->GetSubsystem<UNsUISubsystem>()->CloseScreen(this); }
}

bool UNsUISmokeTestRootWidget::HandleNsScreenCancel_Implementation() { Close(); return true; }

void UNsUISmokeTestRootWidget::OpenMenu()
{
    GetOwningLocalPlayer()->GetSubsystem<UNsUISubsystem>()->OpenScreen(StaticClass(), nullptr, ENsUILayer::Menu);
}
void UNsUISmokeTestRootWidget::OpenModal()
{
    GetOwningLocalPlayer()->GetSubsystem<UNsUISubsystem>()->OpenScreen(StaticClass(), nullptr, ENsUILayer::Modal);
}
void UNsUISmokeTestRootWidget::OpenExternal()
{
    CreateWidget<UNsUISmokeTestRootWidget>(GetOwningPlayer())->AddToViewport(100);
}
void UNsUISmokeTestRootWidget::AddControl()
{
    // Cross a WidgetTree boundary when discovering the new control.
    UUserWidget* Nested = CreateWidget<UUserWidget>(GetOwningPlayer());
    UButton* Button = Nested->WidgetTree->ConstructWidget<UButton>();
    UTextBlock* Text = Nested->WidgetTree->ConstructWidget<UTextBlock>();
    Text->SetText(FText::FromString(TEXT("Dynamic control")));
    Button->SetContent(Text);
    Nested->WidgetTree->RootWidget = Button;
    Button->OnClicked.AddUniqueDynamic(this, &ThisClass::RemoveControl);
    Controls->AddChildToVerticalBox(Nested);
}
void UNsUISmokeTestRootWidget::RemoveControl()
{
    if (Controls->GetChildrenCount() > 8) { Controls->RemoveChildAt(Controls->GetChildrenCount() - 1); }
}
void UNsUISmokeTestRootWidget::OpenActors()
{
    UNsUISmokeWorldWidget* Widget = CreateWidget<UNsUISmokeWorldWidget>(GetOwningPlayer());
    Widget->bActors = true;
    Widget->AddToViewport(100);
}
void UNsUISmokeTestRootWidget::OpenLocations()
{
    UNsUISmokeWorldWidget* Widget = CreateWidget<UNsUISmokeWorldWidget>(GetOwningPlayer());
    Widget->bActors = false;
    Widget->AddToViewport(100);
}
FNsUIScreenConfig UNsUISmokeWorldWidget::GetNsScreenConfig_Implementation() const
{
    FNsUIScreenConfig Config;
    Config.SelectionMode = bActors ? UNsUIActorSelectionMode::StaticClass() : UNsUILocationSelectionMode::StaticClass();
    return Config;
}
void UNsUISmokeWorldWidget::OnNsScreenOpened_Implementation(UObject*, UNsUISelectionMode* Mode)
{
    Selection = Mode;
    FVector Origin;
    FRotator Rotation;
    GetOwningPlayer()->GetPlayerViewPoint(Origin, Rotation);
    TArray<FNsUILocation> Locations;
    for (int32 Index = 0; Index < 3; ++Index)
    {
        const FVector Position = Origin + Rotation.RotateVector(FVector(500, (Index - 1) * 150, 0));
        Locations.Add({FName(*FString::Printf(TEXT("Location%d"), Index)), Position});
        if (bActors)
        {
            FActorSpawnParameters Params;
            Params.ObjectFlags |= RF_Transient;
            Actors.Add(GetWorld()->SpawnActor<ATargetPoint>(ATargetPoint::StaticClass(), Position, FRotator::ZeroRotator, Params));
        }
    }
    if (auto* ActorMode = Cast<UNsUIActorSelectionMode>(Mode)) { ActorMode->SetCandidates(Actors); }
    if (auto* LocationMode = Cast<UNsUILocationSelectionMode>(Mode)) { LocationMode->SetCandidates(Locations); }
}
void UNsUISmokeWorldWidget::OnNsScreenClosed_Implementation()
{
    for (const auto& Actor : Actors) { if (Actor.IsValid()) { Actor->Destroy(); } }
    Actors.Reset();
    Selection = nullptr;
}
bool UNsUISmokeWorldWidget::HandleNsScreenCancel_Implementation() { RemoveFromParent(); return true; }
bool UNsUISmokeWorldWidget::HandleNsScreenConfirm_Implementation()
{
    if (auto* Mode = Cast<UNsUIActorSelectionMode>(Selection))
    {
        if (AActor* Actor = Mode->GetSelectedActor()) { Actor->Destroy(); }
    }
    return true;
}
