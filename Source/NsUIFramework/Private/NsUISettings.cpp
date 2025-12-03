// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUISettings.h"


UNsUISettings* UNsUISettings::GetSettings()
{
    return GetMutableDefault<UNsUISettings>();
}
