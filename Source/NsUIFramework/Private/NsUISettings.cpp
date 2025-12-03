// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUISettings.h"
#include "NsUICursor.h"


UNsUISettings::UNsUISettings(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CursorWidgetClass = UNsUICursor::StaticClass();
}

UNsUISettings* UNsUISettings::GetSettings()
{
    return GetMutableDefault<UNsUISettings>();
}
