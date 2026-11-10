#include "NsUIWidget.h"

#include "Engine/LocalPlayer.h"
#include "NsUISubsystem.h"

void UNsUIWidget::RemoveFromParent()
{
    // An open widget closes through the framework, so the stack and the input stay correct
    const ULocalPlayer* const Player = GetOwningLocalPlayer();
    UNsUISubsystem* const UI = Player != nullptr ? Player->GetSubsystem<UNsUISubsystem>() : nullptr;
    if (UI != nullptr && UI->IsWidgetOpen(this) && !UI->IsExternalWidget(this))
    {
        UI->CloseWidget(this);
        return;
    }
    if (UI != nullptr)
    {
        UI->RemoveExternalWidget(this);
    }
    Super::RemoveFromParent();
}
