#pragma once

#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/NavigationConfig.h"

class UNsUISubsystem;

TSharedPtr<IInputProcessor> CreateNsUINavigationInputPreProcessor(UNsUISubsystem* Subsystem);
TSharedRef<FNavigationConfig> CreateNsUINavigationConfig(UNsUISubsystem* Subsystem, const TSharedRef<FNavigationConfig>& WrappedConfig);

