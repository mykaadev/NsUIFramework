#include "NsUISettings.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "NsUICursorWidget.h"

UNsUISettings::UNsUISettings()
{
    CategoryName = TEXT("Plugins");
    InputMappingPriority = 0;
    RootZOrder = 10;
    CursorZOrder = 1000;
    PointerPickRadius = 24.f;
    WorldNavigationConeDegrees = 60.f;
}

const UInputAction* UNsUISettings::GetInputAction(const ENsUIAction Action) const
{
    const TSoftObjectPtr<UInputAction>* const Found = InputActions.Find(Action);
    return Found != nullptr ? Found->LoadSynchronous() : nullptr;
}
